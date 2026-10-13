#include "Value.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <sstream>
#include <unordered_set>

using namespace std;

namespace {
    enum class NumericOrder {
        Less,
        Equal,
        Greater,
        Unordered
    };

    using ContainerPairs = vector<pair<const void*, const void*>>;

    //checks numeric equality types
    bool isNumeric(const Value& value) {
        return holds_alternative<int64_t>(value) ||
            holds_alternative<double>(value) ||
            holds_alternative<bool>(value);
    }

    int64_t integerValue(const Value& value) {
        if (holds_alternative<bool>(value)) {
            return get<bool>(value) ? 1 : 0;
        }
        return get<int64_t>(value);
    }

    //avoids rounding large integers when comparing with floats
    NumericOrder compareIntegerFloat(int64_t integer, double floating) {
        if (isnan(floating)) {
            return NumericOrder::Unordered;
        }
        if (floating < -0x1p63) {
            return NumericOrder::Greater;
        }
        if (floating >= 0x1p63) {
            return NumericOrder::Less;
        }

        int64_t truncated = static_cast<int64_t>(floating);
        if (integer < truncated) {
            return NumericOrder::Less;
        }
        if (integer > truncated) {
            return NumericOrder::Greater;
        }
        if (floating == trunc(floating)) {
            return NumericOrder::Equal;
        }
        return floating > 0 ? NumericOrder::Less : NumericOrder::Greater;
    }

    NumericOrder compareNumbers(const Value& left, const Value& right) {
        bool leftFloat = holds_alternative<double>(left);
        bool rightFloat = holds_alternative<double>(right);
        if (!leftFloat && !rightFloat) {
            int64_t a = integerValue(left);
            int64_t b = integerValue(right);
            return a < b ? NumericOrder::Less :
                a > b ? NumericOrder::Greater : NumericOrder::Equal;
        }
        if (!leftFloat) {
            return compareIntegerFloat(integerValue(left), get<double>(right));
        }
        if (!rightFloat) {
            NumericOrder order = compareIntegerFloat(integerValue(right), get<double>(left));
            if (order == NumericOrder::Less) {
                return NumericOrder::Greater;
            }
            if (order == NumericOrder::Greater) {
                return NumericOrder::Less;
            }
            return order;
        }

        double a = get<double>(left);
        double b = get<double>(right);
        if (isnan(a) || isnan(b)) {
            return NumericOrder::Unordered;
        }
        return a < b ? NumericOrder::Less :
            a > b ? NumericOrder::Greater : NumericOrder::Equal;
    }

    //returns shared identity for recursive container operations
    const void* containerAddress(const Value& value) {
        if (holds_alternative<shared_ptr<ListValue>>(value)) {
            return get<shared_ptr<ListValue>>(value).get();
        }
        if (holds_alternative<shared_ptr<SetValue>>(value)) {
            return get<shared_ptr<SetValue>>(value).get();
        }
        if (holds_alternative<shared_ptr<DictValue>>(value)) {
            return get<shared_ptr<DictValue>>(value).get();
        }
        return nullptr;
    }

    bool equalValues(const Value& left, const Value& right, ContainerPairs& active);

    bool equalElements(const vector<Value>& left, const vector<Value>& right, ContainerPairs& active) {
        if (left.size() != right.size()) {
            return false;
        }
        for (size_t i = 0; i < left.size(); i++) {
            if (!equalValues(left[i], right[i], active)) {
                return false;
            }
        }
        return true;
    }

    bool equalSets(const vector<Value>& left, const vector<Value>& right, ContainerPairs& active) {
        if (left.size() != right.size()) {
            return false;
        }
        vector<bool> matched(right.size(), false);
        for (const Value& element : left) {
            bool found = false;
            for (size_t i = 0; i < right.size(); i++) {
                if (!matched[i] && equalValues(element, right[i], active)) {
                    matched[i] = true;
                    found = true;
                    break;
                }
            }
            if (!found) {
                return false;
            }
        }
        return true;
    }

    bool equalEntries(const vector<pair<Value, Value>>& left,
                      const vector<pair<Value, Value>>& right, ContainerPairs& active) {
        if (left.size() != right.size()) {
            return false;
        }
        vector<bool> matched(right.size(), false);
        for (const auto& entry : left) {
            bool found = false;
            for (size_t i = 0; i < right.size(); i++) {
                if (!matched[i] && equalValues(entry.first, right[i].first, active) &&
                    equalValues(entry.second, right[i].second, active)) {
                    matched[i] = true;
                    found = true;
                    break;
                }
            }
            if (!found) {
                return false;
            }
        }
        return true;
    }

    bool equalValues(const Value& left, const Value& right, ContainerPairs& active) {
        if (isNumeric(left) && isNumeric(right)) {
            return compareNumbers(left, right) == NumericOrder::Equal;
        }
        if (left.index() != right.index()) {
            return false;
        }
        if (holds_alternative<string>(left)) {
            return get<string>(left) == get<string>(right);
        }
        if (holds_alternative<monostate>(left)) {
            return true;
        }

        pair<const void*, const void*> addresses = {containerAddress(left), containerAddress(right)};
        if (addresses.first == addresses.second || find(active.begin(), active.end(), addresses) != active.end()) {
            return true;
        }
        active.push_back(addresses);
        bool equal = false;
        if (holds_alternative<shared_ptr<ListValue>>(left)) {
            equal = equalElements(get<shared_ptr<ListValue>>(left)->elements,
                                  get<shared_ptr<ListValue>>(right)->elements, active);
        } else if (holds_alternative<shared_ptr<SetValue>>(left)) {
            equal = equalSets(get<shared_ptr<SetValue>>(left)->elements,
                              get<shared_ptr<SetValue>>(right)->elements, active);
        } else if (holds_alternative<shared_ptr<DictValue>>(left)) {
            equal = equalEntries(get<shared_ptr<DictValue>>(left)->entries,
                                 get<shared_ptr<DictValue>>(right)->entries, active);
        }
        active.pop_back();
        return equal;
    }

    bool lessValues(const Value& left, const Value& right, ContainerPairs& active) {
        if (isNumeric(left) && isNumeric(right)) {
            return compareNumbers(left, right) == NumericOrder::Less;
        }
        if (holds_alternative<string>(left) && holds_alternative<string>(right)) {
            return get<string>(left) < get<string>(right);
        }

        const vector<Value>* leftElements = nullptr;
        const vector<Value>* rightElements = nullptr;
        if (holds_alternative<shared_ptr<ListValue>>(left) && holds_alternative<shared_ptr<ListValue>>(right)) {
            leftElements = &get<shared_ptr<ListValue>>(left)->elements;
            rightElements = &get<shared_ptr<ListValue>>(right)->elements;
        } else {
            throw runtime_error("Values are not ordered");
        }

        pair<const void*, const void*> addresses = {containerAddress(left), containerAddress(right)};
        if (addresses.first == addresses.second) {
            return false;
        }
        if (find(active.begin(), active.end(), addresses) != active.end()) {
            throw runtime_error("Values are not ordered");
        }
        active.push_back(addresses);
        for (size_t i = 0; i < min(leftElements->size(), rightElements->size()); i++) {
            ContainerPairs equalityActive;
            if (!equalValues((*leftElements)[i], (*rightElements)[i], equalityActive)) {
                bool less = lessValues((*leftElements)[i], (*rightElements)[i], active);
                active.pop_back();
                return less;
            }
        }
        active.pop_back();
        return leftElements->size() < rightElements->size();
    }

    void freezeContents(const Value& value, unordered_set<const void*>& visited) {
        const void* address = containerAddress(value);
        if (!address || !visited.insert(address).second) {
            return;
        }

        const vector<Value>* elements = nullptr;
        if (holds_alternative<shared_ptr<ListValue>>(value)) {
            auto list = get<shared_ptr<ListValue>>(value);
            list->frozen = true;
            elements = &list->elements;
        } else if (holds_alternative<shared_ptr<SetValue>>(value)) {
            auto set = get<shared_ptr<SetValue>>(value);
            set->frozen = true;
            elements = &set->elements;
        } else if (holds_alternative<shared_ptr<DictValue>>(value)) {
            auto dict = get<shared_ptr<DictValue>>(value);
            dict->frozen = true;
            for (const auto& entry : dict->entries) {
                freezeContents(entry.first, visited);
                freezeContents(entry.second, visited);
            }
        }
        if (elements) {
            for (const Value& element : *elements) {
                freezeContents(element, visited);
            }
        }
    }

    //keeps string boundaries visible inside containers
    string quoteString(const string& value) {
        string quoted = "\"";
        const char* hex = "0123456789abcdef";
        for (unsigned char character : value) {
            switch (character) {
                case '\\': quoted += "\\\\"; break;
                case '"': quoted += "\\\""; break;
                case '\n': quoted += "\\n"; break;
                case '\r': quoted += "\\r"; break;
                case '\t': quoted += "\\t"; break;
                default:
                    if (character < 0x20 || character == 0x7f) {
                        quoted += "\\x";
                        quoted += hex[character >> 4];
                        quoted += hex[character & 0xf];
                    } else {
                        quoted += static_cast<char>(character);
                    }
                    break;
            }
        }
        return quoted + "\"";
    }

    string stringifyValue(const Value& value, bool nested, unordered_set<const void*>& active) {
        if (holds_alternative<int64_t>(value)) {
            return to_string(get<int64_t>(value));
        }
        if (holds_alternative<double>(value)) {
            ostringstream stream;
            stream << get<double>(value);
            return stream.str();
        }
        if (holds_alternative<string>(value)) {
            return nested ? quoteString(get<string>(value)) : get<string>(value);
        }
        if (holds_alternative<bool>(value)) {
            return get<bool>(value) ? "true" : "false";
        }
        if (holds_alternative<monostate>(value)) {
            return "none";
        }

        const vector<Value>* elements = nullptr;
        string opening;
        string closing;
        if (holds_alternative<shared_ptr<ListValue>>(value)) {
            elements = &get<shared_ptr<ListValue>>(value)->elements;
            opening = "[";
            closing = "]";
        } else if (holds_alternative<shared_ptr<SetValue>>(value)) {
            elements = &get<shared_ptr<SetValue>>(value)->elements;
            opening = "<";
            closing = ">";
        } else {
            opening = "{";
            closing = "}";
        }

        const void* address = containerAddress(value);
        if (!active.insert(address).second) {
            return opening + "..." + closing;
        }
        string result = opening;
        if (elements) {
            for (size_t i = 0; i < elements->size(); i++) {
                if (i > 0) {
                    result += ", ";
                }
                result += stringifyValue((*elements)[i], true, active);
            }
        } else {
            const auto& entries = get<shared_ptr<DictValue>>(value)->entries;
            for (size_t i = 0; i < entries.size(); i++) {
                if (i > 0) {
                    result += ", ";
                }
                result += stringifyValue(entries[i].first, true, active) + ": " +
                    stringifyValue(entries[i].second, true, active);
            }
        }
        active.erase(address);
        return result + closing;
    }
}

//return data type of a value
ValueType getValueType(const Value& value) {
    if (holds_alternative<int64_t>(value)) {
        return ValueType::Int;
    }
    if (holds_alternative<double>(value)) {
        return ValueType::Float;
    }
    if (holds_alternative<string>(value)) {
        return ValueType::String;
    }
    if (holds_alternative<bool>(value)) {
        return ValueType::Bool;
    }
    if (holds_alternative<monostate>(value)) {
        return ValueType::None;
    }
    if (holds_alternative<shared_ptr<ListValue>>(value)) {
        return ValueType::List;
    }
    if (holds_alternative<shared_ptr<SetValue>>(value)) {
        return ValueType::Set;
    }
    if (holds_alternative<shared_ptr<DictValue>>(value)) {
        return ValueType::Dict;
    }
    throw runtime_error("Unknown value type");
}

//converts type to readable string
string valueTypeToString(ValueType type) {
    switch (type) {
        case ValueType::Int: return "int";
        case ValueType::Float: return "float";
        case ValueType::String: return "string";
        case ValueType::Bool: return "bool";
        case ValueType::None: return "none";
        case ValueType::List: return "list";
        case ValueType::Set: return "set";
        case ValueType::Dict: return "dict";
        case ValueType::Void: return "void";
    }
    throw runtime_error("Unknown value type");
}

//converts value to readable string
string valueToString(const Value& value) {
    unordered_set<const void*> active;
    return stringifyValue(value, false, active);
}

//compares values and container contents
bool valuesEqual(const Value& left, const Value& right) {
    ContainerPairs active;
    return equalValues(left, right, active);
}

//compares ordered values
bool valueLess(const Value& left, const Value& right) {
    ContainerPairs active;
    return lessValues(left, right, active);
}

//checks dictionary keys and set elements
void requireHashable(const Value& value) {
    ValueType type = getValueType(value);
    if (type == ValueType::List || type == ValueType::Set || type == ValueType::Dict) {
        throw runtime_error("Unhashable type: " + valueTypeToString(type));
    }
}

//freezes a container and all nested contents
void freezeValue(const Value& value) {
    unordered_set<const void*> visited;
    freezeContents(value, visited);
}

//checks whether container contents can change
void requireMutable(const Value& value) {
    bool frozen = false;
    if (holds_alternative<shared_ptr<ListValue>>(value)) {
        frozen = get<shared_ptr<ListValue>>(value)->frozen;
    } else if (holds_alternative<shared_ptr<SetValue>>(value)) {
        frozen = get<shared_ptr<SetValue>>(value)->frozen;
    } else if (holds_alternative<shared_ptr<DictValue>>(value)) {
        frozen = get<shared_ptr<DictValue>>(value)->frozen;
    } else {
        throw runtime_error("Cannot modify " + valueTypeToString(getValueType(value)));
    }
    if (frozen) {
        throw runtime_error("Cannot modify frozen " + valueTypeToString(getValueType(value)));
    }
}
