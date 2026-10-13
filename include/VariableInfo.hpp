#pragma once

#include <optional>
#include "Value.hpp"

//stores variable type, value and mutability
struct VariableInfo {
    ValueType type;
    std::optional<Value> value;
    bool isConst = false;
};