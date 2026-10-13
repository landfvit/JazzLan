#pragma once

#include <cstdint>
#include <string>
#include <variant>

//supported data types
enum class ValueType {
    Int,
    Float,
    String,
    Bool
};

//runtime value representation
using Value = std::variant<
    std::int64_t,
    double,
    std::string,
    bool
>;

//returns data type of a value
ValueType getValueType(const Value& value);
//converts value to readable string
std::string valueToString(const Value& value);
