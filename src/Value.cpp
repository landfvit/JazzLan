#include "Value.hpp"

#include <stdexcept>
#include <sstream>

using namespace std;

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
    throw runtime_error("Unknwon value type");
}
//converts value to readable string
string valueToString(const Value& value) {
    if (holds_alternative<int64_t>(value)) {
        return to_string(get<int64_t>(value));
    }
    if (holds_alternative<double>(value)) {
        ostringstream stream;
        stream << get<double>(value);
        return stream.str();
    }
    if (holds_alternative<string>(value)) {
        return get<string>(value);
    }
    if (holds_alternative<bool>(value)) {
        return get<bool>(value) ? "true" : "false";
    }
    throw runtime_error("Unknown value type");
}
