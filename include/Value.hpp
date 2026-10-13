#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <vector>

//supported data types
enum class ValueType {
    Int,
    Float,
    String,
    Bool,
    None,
    List,
    Set,
    Dict,
    Void //function return annotation only
};

struct ListValue;
struct SetValue;
struct DictValue;

//runtime value representation
using Value = std::variant<
    std::int64_t,
    double,
    std::string,
    bool,
    std::monostate,
    std::shared_ptr<ListValue>,
    std::shared_ptr<SetValue>,
    std::shared_ptr<DictValue>
>;

//shared container contents
struct ListValue {
    std::vector<Value> elements;
    bool frozen = false;
};

struct SetValue {
    std::vector<Value> elements;
    bool frozen = false;
};

struct DictValue {
    std::vector<std::pair<Value, Value>> entries;
    bool frozen = false;
};

//returns data type of a value
ValueType getValueType(const Value& value);
//converts type to readable string
std::string valueTypeToString(ValueType type);
//converts value to readable string
std::string valueToString(const Value& value);
//compares values and container contents
bool valuesEqual(const Value& left, const Value& right);
//compares ordered values
bool valueLess(const Value& left, const Value& right);
//checks dictionary keys and set elements
void requireHashable(const Value& value);
//freezes a container and all nested contents
void freezeValue(const Value& value);
//checks whether container contents can change
void requireMutable(const Value& value);
