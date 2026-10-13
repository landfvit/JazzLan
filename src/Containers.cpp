#include "Containers.hpp"

#include <limits>
#include <stdexcept>
#include <utility>

using namespace std;

//breaks remaining cycles when the interpreter finishes
ContainerHeap::~ContainerHeap() {
    for (const auto& object : objects) {
        visit([](const auto& pointer) {
            if (auto value = pointer.lock()) {
                if constexpr (is_same_v<decltype(value), shared_ptr<DictValue>>) {
                    value->entries.clear();
                } else {
                    value->elements.clear();
                }
            }
        }, object);
    }
}

//checks a builtin argument count
static void checkArguments(
    const string& name,
    const vector<Value>& arguments,
    size_t minimum,
    size_t maximum
) {
    if (arguments.size() < minimum || arguments.size() > maximum) {
        throw runtime_error("Invalid argument count: " + name);
    }
}

//accepts python-style integer indexes without changing value types
static int64_t integerIndex(const Value& value) {
    if (holds_alternative<int64_t>(value)) {
        return get<int64_t>(value);
    }
    if (holds_alternative<bool>(value)) {
        return get<bool>(value) ? 1 : 0;
    }
    throw runtime_error("Index must be int or bool");
}

//normalizes an index relative to a sequence
static size_t itemIndex(const Value& value, size_t size) {
    int64_t index = integerIndex(value);
    if (index < 0) {
        index += static_cast<int64_t>(size);
    }
    if (index < 0 || static_cast<uint64_t>(index) >= size) {
        throw runtime_error("Index out of range");
    }
    return static_cast<size_t>(index);
}

//finds an equal value without conflating different numeric values
static size_t findElement(const vector<Value>& values, const Value& value) {
    for (size_t i = 0; i < values.size(); i++) {
        if (valuesEqual(values[i], value)) {
            return i;
        }
    }
    return values.size();
}

//finds a dictionary key in insertion order
static size_t findKey(const DictValue& dictionary, const Value& key) {
    requireHashable(key);
    for (size_t i = 0; i < dictionary.entries.size(); i++) {
        if (valuesEqual(dictionary.entries[i].first, key)) {
            return i;
        }
    }
    return dictionary.entries.size();
}

//inserts a unique set element
static void addElement(SetValue& set, const Value& value) {
    requireHashable(value);
    if (findElement(set.elements, value) == set.elements.size()) {
        set.elements.push_back(value);
    }
}

//keeps the original key and position when replacing a dictionary value
static void putEntry(DictValue& dictionary, const Value& key, const Value& value) {
    size_t index = findKey(dictionary, key);
    if (index == dictionary.entries.size()) {
        dictionary.entries.emplace_back(key, value);
    } else {
        dictionary.entries[index].second = value;
    }
}

//splits utf-8 strings into characters for indexing and iteration
static vector<Value> stringCharacters(const string& text) {
    vector<Value> result;
    for (size_t i = 0; i < text.size();) {
        size_t end = i + 1;
        while (end < text.size() && (static_cast<unsigned char>(text[end]) & 0xc0) == 0x80) {
            end++;
        }
        result.emplace_back(text.substr(i, end - i));
        i = end;
    }
    return result;
}

//copies iteration values so self-extension does not invalidate references
vector<Value> iterableValues(const Value& value) {
    if (const auto* list = get_if<shared_ptr<ListValue>>(&value)) {
        return (*list)->elements;
    }
    if (const auto* set = get_if<shared_ptr<SetValue>>(&value)) {
        return (*set)->elements;
    }
    if (const auto* dictionary = get_if<shared_ptr<DictValue>>(&value)) {
        vector<Value> keys;
        for (const auto& entry : (*dictionary)->entries) {
            keys.push_back(entry.first);
        }
        return keys;
    }
    if (const auto* text = get_if<string>(&value)) {
        return stringCharacters(*text);
    }
    throw runtime_error("Value is not iterable: " + valueTypeToString(getValueType(value)));
}

//reads the size directly from shared storage
size_t containerSize(const Value& value) {
    if (const auto* list = get_if<shared_ptr<ListValue>>(&value)) return (*list)->elements.size();
    if (const auto* set = get_if<shared_ptr<SetValue>>(&value)) return (*set)->elements.size();
    if (const auto* dictionary = get_if<shared_ptr<DictValue>>(&value)) return (*dictionary)->entries.size();
    if (const auto* text = get_if<string>(&value)) {
        return count_if(text->begin(), text->end(), [](unsigned char character) {
            return (character & 0xc0) != 0x80;
        });
    }
    throw runtime_error("Value is not iterable: " + valueTypeToString(getValueType(value)));
}

//reads dictionary input from another mapping or iterable pairs
static vector<pair<Value, Value>> dictionaryEntries(const Value& value) {
    if (const auto* dictionary = get_if<shared_ptr<DictValue>>(&value)) {
        return (*dictionary)->entries;
    }
    vector<pair<Value, Value>> entries;
    for (const auto& item : iterableValues(value)) {
        auto pair = iterableValues(item);
        if (pair.size() != 2) {
            throw runtime_error("Dictionary entries must contain two values");
        }
        requireHashable(pair[0]);
        entries.emplace_back(pair[0], pair[1]);
    }
    return entries;
}

//constructs builtin containers and computes their length
Value callContainerFunction(
    const string& name,
    const vector<Value>& arguments,
    ContainerHeap& heap
) {
    checkArguments(name, arguments, name == "len" ? 1 : 0, 1);

    if (name == "len") {
        return static_cast<int64_t>(containerSize(arguments[0]));
    }
    if (name == "dict") {
        auto dictionary = heap.create<DictValue>();
        if (!arguments.empty()) {
            for (const auto& entry : dictionaryEntries(arguments[0])) {
                putEntry(*dictionary, entry.first, entry.second);
            }
        }
        return dictionary;
    }
    vector<Value> elements;
    if (!arguments.empty()) {
        elements = iterableValues(arguments[0]);
    }
    if (name == "list") {
        auto list = heap.create<ListValue>();
        list->elements = std::move(elements);
        return list;
    }
    if (name == "set") {
        auto set = heap.create<SetValue>();
        for (const auto& element : elements) {
            addElement(*set, element);
        }
        return set;
    }
    throw runtime_error("Unknown builtin: " + name);
}

//membership tests compare elements, dictionary keys or substrings
bool containsValue(const Value& container, const Value& value) {
    if (const auto* dictionary = get_if<shared_ptr<DictValue>>(&container)) {
        return findKey(**dictionary, value) != (*dictionary)->entries.size();
    }
    if (const auto* text = get_if<string>(&container)) {
        if (!holds_alternative<string>(value)) {
            throw runtime_error("String membership requires a string");
        }
        return text->find(get<string>(value)) != string::npos;
    }
    if (const auto* set = get_if<shared_ptr<SetValue>>(&container)) {
        requireHashable(value);
        return findElement((*set)->elements, value) != (*set)->elements.size();
    }
    if (const auto* list = get_if<shared_ptr<ListValue>>(&container)) {
        return findElement((*list)->elements, value) != (*list)->elements.size();
    }
    auto elements = iterableValues(container);
    return findElement(elements, value) != elements.size();
}

//reads an item without copying a nested container
Value getItem(const Value& object, const Value& index) {
    if (const auto* list = get_if<shared_ptr<ListValue>>(&object)) {
        return (*list)->elements[itemIndex(index, (*list)->elements.size())];
    }
    if (const auto* dictionary = get_if<shared_ptr<DictValue>>(&object)) {
        size_t position = findKey(**dictionary, index);
        if (position == (*dictionary)->entries.size()) {
            throw runtime_error("Key not found: " + valueToString(index));
        }
        return (*dictionary)->entries[position].second;
    }
    if (holds_alternative<shared_ptr<SetValue>>(object)) {
        throw runtime_error("Set does not support indexing");
    }
    auto elements = iterableValues(object);
    return elements[itemIndex(index, elements.size())];
}

//writes only to mutable list or dictionary objects
void setItem(const Value& object, const Value& index, const Value& value) {
    requireMutable(object);
    if (const auto* list = get_if<shared_ptr<ListValue>>(&object)) {
        (*list)->elements[itemIndex(index, (*list)->elements.size())] = value;
        return;
    }
    if (const auto* dictionary = get_if<shared_ptr<DictValue>>(&object)) {
        putEntry(**dictionary, index, value);
        return;
    }
    throw runtime_error("Value does not support item assignment");
}

//clamps an explicit slice bound while preserving omitted negative bounds
static int64_t sliceBound(const Value& value, int64_t size, bool reverse) {
    int64_t bound = integerIndex(value);
    if (bound < 0) {
        bound += size;
    }
    return clamp(bound, reverse ? int64_t{-1} : int64_t{0}, reverse ? size - 1 : size);
}

Value sliceValue(
    const Value& object,
    const optional<Value>& start,
    const optional<Value>& stop,
    const optional<Value>& step,
    ContainerHeap& heap
) {
    ValueType type = getValueType(object);
    if (type != ValueType::List && type != ValueType::String) {
        throw runtime_error("Value does not support slicing");
    }
    auto elements = iterableValues(object);
    int64_t size = static_cast<int64_t>(elements.size());
    bool hasStart = start.has_value() && !holds_alternative<monostate>(start.value());
    bool hasStop = stop.has_value() && !holds_alternative<monostate>(stop.value());
    bool hasStep = step.has_value() && !holds_alternative<monostate>(step.value());
    int64_t stride = hasStep ? integerIndex(step.value()) : 1;
    if (stride == 0) {
        throw runtime_error("Slice step cannot be zero");
    }
    bool reverse = stride < 0;
    int64_t first = hasStart ? sliceBound(start.value(), size, reverse) : (reverse ? size - 1 : 0);
    int64_t last = hasStop ? sliceBound(stop.value(), size, reverse) : (reverse ? -1 : size);
    vector<Value> result;
    for (int64_t i = first; reverse ? i > last : i < last;) {
        result.push_back(elements[static_cast<size_t>(i)]);
        if (reverse ? stride <= last - i : stride >= last - i) {
            break;
        }
        i += stride;
    }
    if (type == ValueType::String) {
        string text;
        for (const auto& character : result) {
            text += get<string>(character);
        }
        return text;
    }
    auto list = heap.create<ListValue>();
    list->elements = std::move(result);
    return list;
}

//computes set operations without mutating either input
static vector<Value> setElements(
    const vector<Value>& left,
    const string& op,
    const vector<Value>& right
) {
    SetValue result;
    for (const auto& value : right) {
        requireHashable(value);
    }
    for (const auto& value : left) {
        bool present = findElement(right, value) != right.size();
        if (op == "|" || (op == "&" && present) || ((op == "-" || op == "^") && !present)) {
            addElement(result, value);
        }
    }
    if (op == "|" || op == "^") {
        for (const auto& value : right) {
            if (op == "|" || findElement(left, value) == left.size()) {
                addElement(result, value);
            }
        }
    }
    return result.elements;
}

//bounded stable merge sort also tolerates unordered numeric values
static void sortSequence(vector<Value>& values, vector<Value>& buffer, size_t first, size_t last) {
    if (last - first < 2) {
        return;
    }
    size_t middle = first + (last - first) / 2;
    sortSequence(values, buffer, first, middle);
    sortSequence(values, buffer, middle, last);

    size_t left = first;
    size_t right = middle;
    for (size_t i = first; i < last; i++) {
        if (left == middle || (right < last && valueLess(values[right], values[left]))) {
            buffer[i] = values[right++];
        } else {
            buffer[i] = values[left++];
        }
    }
    for (size_t i = first; i < last; i++) {
        values[i] = std::move(buffer[i]);
    }
}

//implements list methods
static Value listMethod(
    const Value& object,
    const string& name,
    const vector<Value>& arguments,
    ContainerHeap& heap
) {
    const auto& elements = get<shared_ptr<ListValue>>(object)->elements;
    if (name == "count") {
        checkArguments(name, arguments, 1, 1);
        int64_t count = 0;
        for (const auto& value : elements) {
            if (valuesEqual(value, arguments[0])) count++;
        }
        return count;
    }
    if (name == "index") {
        checkArguments(name, arguments, 1, 3);
        int64_t size = static_cast<int64_t>(elements.size());
        int64_t start = arguments.size() > 1 ? sliceBound(arguments[1], size, false) : 0;
        int64_t stop = arguments.size() > 2 ? sliceBound(arguments[2], size, false) : size;
        for (int64_t i = start; i < stop; i++) {
            if (valuesEqual(elements[static_cast<size_t>(i)], arguments[0])) return i;
        }
        throw runtime_error("Value not found");
    }
    if (name == "copy") {
        checkArguments(name, arguments, 0, 0);
        return callContainerFunction("list", {object}, heap);
    }
    auto& list = *get<shared_ptr<ListValue>>(object);
    if (name == "append") {
        checkArguments(name, arguments, 1, 1);
        requireMutable(object);
        list.elements.push_back(arguments[0]);
    } else if (name == "extend") {
        checkArguments(name, arguments, 1, 1);
        requireMutable(object);
        auto added = iterableValues(arguments[0]);
        list.elements.insert(list.elements.end(), added.begin(), added.end());
    } else if (name == "insert") {
        checkArguments(name, arguments, 2, 2);
        requireMutable(object);
        int64_t index = sliceBound(arguments[0], static_cast<int64_t>(list.elements.size()), false);
        list.elements.insert(list.elements.begin() + index, arguments[1]);
    } else if (name == "pop") {
        checkArguments(name, arguments, 0, 1);
        requireMutable(object);
        size_t index = itemIndex(arguments.empty() ? Value{int64_t{-1}} : arguments[0], list.elements.size());
        Value result = list.elements[index];
        list.elements.erase(list.elements.begin() + index);
        return result;
    } else if (name == "remove") {
        checkArguments(name, arguments, 1, 1);
        requireMutable(object);
        size_t index = findElement(list.elements, arguments[0]);
        if (index == list.elements.size()) throw runtime_error("Value not found");
        list.elements.erase(list.elements.begin() + index);
    } else if (name == "clear") {
        checkArguments(name, arguments, 0, 0);
        requireMutable(object);
        list.elements.clear();
    } else if (name == "reverse") {
        checkArguments(name, arguments, 0, 0);
        requireMutable(object);
        reverse(list.elements.begin(), list.elements.end());
    } else if (name == "sort") {
        checkArguments(name, arguments, 0, 0);
        requireMutable(object);
        auto sorted = elements;
        vector<Value> buffer(sorted.size());
        sortSequence(sorted, buffer, 0, sorted.size());
        list.elements = std::move(sorted);
    } else {
        throw runtime_error("Unknown list method: " + name);
    }
    return monostate{};
}

//set methods use equality shared with comparisons and dictionary keys
static Value setMethod(
    const Value& object,
    const string& name,
    const vector<Value>& arguments,
    ContainerHeap& heap
) {
    auto& set = *get<shared_ptr<SetValue>>(object);
    if (name == "copy") {
        checkArguments(name, arguments, 0, 0);
        return callContainerFunction("set", {object}, heap);
    }
    if (name == "add") {
        checkArguments(name, arguments, 1, 1);
        requireMutable(object);
        addElement(set, arguments[0]);
    } else if (name == "remove" || name == "discard") {
        checkArguments(name, arguments, 1, 1);
        requireMutable(object);
        requireHashable(arguments[0]);
        size_t index = findElement(set.elements, arguments[0]);
        if (index == set.elements.size()) {
            if (name == "remove") throw runtime_error("Value not found");
        } else {
            set.elements.erase(set.elements.begin() + index);
        }
    } else if (name == "pop") {
        checkArguments(name, arguments, 0, 0);
        requireMutable(object);
        if (set.elements.empty()) throw runtime_error("Cannot pop from empty set");
        Value result = set.elements.back();
        set.elements.pop_back();
        return result;
    } else if (name == "clear") {
        checkArguments(name, arguments, 0, 0);
        requireMutable(object);
        set.elements.clear();
    } else if (name == "isdisjoint" || name == "issubset" || name == "issuperset") {
        checkArguments(name, arguments, 1, 1);
        auto other = iterableValues(arguments[0]);
        for (const auto& value : other) requireHashable(value);
        if (name == "isdisjoint") {
            for (const auto& value : other) {
                if (findElement(set.elements, value) != set.elements.size()) return false;
            }
        } else {
            const auto& left = name == "issubset" ? set.elements : other;
            const auto& right = name == "issubset" ? other : set.elements;
            for (const auto& value : left) {
                if (findElement(right, value) == right.size()) return false;
            }
        }
        return true;
    } else {
        string op;
        bool mutate = false;
        if (name == "union" || name == "update") op = "|";
        else if (name == "intersection" || name == "intersection_update") op = "&";
        else if (name == "difference" || name == "difference_update") op = "-";
        else if (name == "symmetric_difference" || name == "symmetric_difference_update") op = "^";
        else throw runtime_error("Unknown set method: " + name);

        mutate = name == "update" || name.ends_with("_update");
        checkArguments(name, arguments, op == "^" ? 1 : 0, op == "^" ? 1 : numeric_limits<size_t>::max());
        if (mutate) requireMutable(object);
        auto result = set.elements;
        for (const auto& argument : arguments) {
            result = setElements(result, op, iterableValues(argument));
        }
        if (mutate) {
            set.elements = std::move(result);
        } else {
            auto copy = heap.create<SetValue>();
            copy->elements = std::move(result);
            return copy;
        }
    }
    return monostate{};
}

//dictionary methods preserve insertion order
static Value dictionaryMethod(
    const Value& object,
    const string& name,
    const vector<Value>& arguments,
    ContainerHeap& heap
) {
    auto& dictionary = *get<shared_ptr<DictValue>>(object);
    if (name == "copy") {
        checkArguments(name, arguments, 0, 0);
        return callContainerFunction("dict", {object}, heap);
    }
    if (name == "keys" || name == "values" || name == "items") {
        checkArguments(name, arguments, 0, 0);
        auto list = heap.create<ListValue>();
        for (const auto& entry : dictionary.entries) {
            if (name == "keys") list->elements.push_back(entry.first);
            else if (name == "values") list->elements.push_back(entry.second);
            else {
                auto pair = heap.create<ListValue>();
                pair->elements = {entry.first, entry.second};
                list->elements.push_back(pair);
            }
        }
        return list;
    }
    if (name == "get" || name == "pop" || name == "setdefault") {
        checkArguments(name, arguments, 1, 2);
        if (name != "get") requireMutable(object);
        size_t index = findKey(dictionary, arguments[0]);
        if (index != dictionary.entries.size()) {
            Value result = dictionary.entries[index].second;
            if (name == "pop") dictionary.entries.erase(dictionary.entries.begin() + index);
            return result;
        }
        if (name == "pop" && arguments.size() == 1) {
            throw runtime_error("Key not found: " + valueToString(arguments[0]));
        }
        Value result = arguments.size() == 2 ? arguments[1] : Value{monostate{}};
        if (name == "setdefault") putEntry(dictionary, arguments[0], result);
        return result;
    }
    if (name == "popitem") {
        checkArguments(name, arguments, 0, 0);
        requireMutable(object);
        if (dictionary.entries.empty()) throw runtime_error("Cannot pop from empty dict");
        auto result = heap.create<ListValue>();
        const auto& entry = dictionary.entries.back();
        result->elements = {entry.first, entry.second};
        dictionary.entries.pop_back();
        return result;
    }
    if (name == "clear") {
        checkArguments(name, arguments, 0, 0);
        requireMutable(object);
        dictionary.entries.clear();
    } else if (name == "update") {
        checkArguments(name, arguments, 0, 1);
        requireMutable(object);
        if (!arguments.empty()) {
            for (const auto& entry : dictionaryEntries(arguments[0])) {
                putEntry(dictionary, entry.first, entry.second);
            }
        }
    } else {
        throw runtime_error("Unknown dict method: " + name);
    }
    return monostate{};
}

Value callContainerMethod(
    const Value& object,
    const string& name,
    const vector<Value>& arguments,
    ContainerHeap& heap
) {
    if (holds_alternative<shared_ptr<ListValue>>(object)) {
        return listMethod(object, name, arguments, heap);
    }
    if (holds_alternative<shared_ptr<SetValue>>(object)) {
        return setMethod(object, name, arguments, heap);
    }
    if (holds_alternative<shared_ptr<DictValue>>(object)) {
        return dictionaryMethod(object, name, arguments, heap);
    }
    throw runtime_error("Value has no container methods");
}

optional<Value> containerOperation(
    const Value& left,
    const string& op,
    const Value& right,
    ContainerHeap& heap
) {
    ValueType leftType = getValueType(left);
    ValueType rightType = getValueType(right);
    if (leftType == ValueType::Set && rightType == ValueType::Set) {
        const auto& a = get<shared_ptr<SetValue>>(left)->elements;
        const auto& b = get<shared_ptr<SetValue>>(right)->elements;
        if (op == "|" || op == "&" || op == "-" || op == "^") {
            auto result = heap.create<SetValue>();
            result->elements = setElements(a, op, b);
            return Value{result};
        }
        if (op == "<" || op == "<=" || op == ">" || op == ">=") {
            const auto& subset = op[0] == '<' ? a : b;
            const auto& superset = op[0] == '<' ? b : a;
            bool result = op.size() == 1 ? subset.size() < superset.size() : subset.size() <= superset.size();
            for (const auto& value : subset) {
                if (findElement(superset, value) == superset.size()) result = false;
            }
            return Value{result};
        }
    }
    if (op == "|" && leftType == ValueType::Dict && rightType == ValueType::Dict) {
        Value result = callContainerFunction("dict", {left}, heap);
        callContainerMethod(result, "update", {right}, heap);
        return result;
    }
    if (op == "+" && leftType == ValueType::List && rightType == ValueType::List) {
        auto values = iterableValues(left);
        auto added = iterableValues(right);
        values.insert(values.end(), added.begin(), added.end());
        auto result = heap.create<ListValue>();
        result->elements = std::move(values);
        return Value{result};
    }
    if (op == "*") {
        bool leftSequence = leftType == ValueType::List;
        bool rightSequence = rightType == ValueType::List;
        if (leftSequence || rightSequence) {
            const Value& sequence = leftSequence ? left : right;
            int64_t count = max(int64_t{0}, integerIndex(leftSequence ? right : left));
            auto source = iterableValues(sequence);
            vector<Value> values;
            if (!source.empty()) {
                if (static_cast<uint64_t>(count) > values.max_size() / source.size()) {
                    throw runtime_error("Repeated sequence is too large");
                }
                values.reserve(source.size() * static_cast<size_t>(count));
                for (int64_t i = 0; i < count; i++) {
                    values.insert(values.end(), source.begin(), source.end());
                }
            }
            auto result = heap.create<ListValue>();
            result->elements = std::move(values);
            return Value{result};
        }
    }
    return nullopt;
}

//evaluates every transformation before changing the original container
Value transformContainer(
    const Value& source,
    const function<Value(const Value&)>& transform,
    ContainerHeap& heap
) {
    ValueType type = getValueType(source);
    if (type == ValueType::Dict) {
        auto entries = get<shared_ptr<DictValue>>(source)->entries;
        auto result = heap.create<DictValue>();
        for (const auto& entry : entries) {
            result->entries.emplace_back(entry.first, transform(entry.second));
        }
        return result;
    }

    if (type != ValueType::List && type != ValueType::Set) {
        throw runtime_error("Transform requires a container");
    }

    auto elements = iterableValues(source);
    if (type == ValueType::Set) {
        auto result = heap.create<SetValue>();
        for (const auto& element : elements) {
            addElement(*result, transform(element));
        }
        return result;
    }

    vector<Value> transformed;
    transformed.reserve(elements.size());
    for (const auto& element : elements) {
        transformed.push_back(transform(element));
    }
    auto result = heap.create<ListValue>();
    result->elements = std::move(transformed);
    return result;
}

//keeps aliases pointing at the transformed object
void replaceContainerContents(const Value& source, const Value& result) {
    requireMutable(source);
    if (getValueType(source) != getValueType(result)) {
        throw runtime_error("Transform result type mismatch");
    }
    if (const auto* list = get_if<shared_ptr<ListValue>>(&source)) {
        (*list)->elements = std::move(get<shared_ptr<ListValue>>(result)->elements);
    } else if (const auto* set = get_if<shared_ptr<SetValue>>(&source)) {
        (*set)->elements = std::move(get<shared_ptr<SetValue>>(result)->elements);
    } else if (const auto* dictionary = get_if<shared_ptr<DictValue>>(&source)) {
        (*dictionary)->entries = std::move(get<shared_ptr<DictValue>>(result)->entries);
    }
}
