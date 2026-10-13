#include "Environment.hpp"

#include <stdexcept>
#include <utility>

using namespace std;

//init environment with optional parent scope
Environment::Environment(Environment* parentEnvironment)
    : parent(parentEnvironment) {
}

//defines variable in current scope
void Environment::define(
    const string& name,
    optional<Value> value,
    optional<ValueType> declaredType,
    bool isConst
) {
    if (variables.contains(name)) {
        throw runtime_error("Variable already defined: " + name);
    }

    //const requires initialization
    if (isConst && !value.has_value()) {
        throw runtime_error("Const requires initialization: " + name);
    }

    ValueType type;

    //explicit type
    if (declaredType.has_value()) {
        type = declaredType.value();
    }

    //inferred type
    else if (value.has_value()) {
        type = getValueType(value.value());
    }

    //type cannot be inferred
    else {
        throw runtime_error("Cannot infer type: " + name);
    }

    //check initial value type
    if (
        value.has_value() &&
        getValueType(value.value()) != type
    ) {
        throw runtime_error("Type mismatch: " + name);
    }

    variables.emplace(
        name,
        VariableInfo{
            type,
            std::move(value),
            isConst
        }
    );
}

//finds variable in current or parent scope
Value Environment::get(const string& name) const {
    auto it = variables.find(name);

    if (it != variables.end()) {
        if (!it->second.value.has_value()) {
            throw runtime_error("Uninitialized variable: " + name);
        }

        return it->second.value.value();
    }

    if (parent != nullptr) {
        return parent->get(name);
    }

    throw runtime_error("Undefined variable: " + name);
}

//updates variable in nearest existing scope
void Environment::assign(const string& name, const Value& value) {
    auto it = variables.find(name);

    if (it != variables.end()) {
        VariableInfo& variable = it->second;

        //check const
        if (variable.isConst) {
            throw runtime_error("Cannot assign to const: " + name);
        }

        //check type
        if (getValueType(value) != variable.type) {
            throw runtime_error("Type mismatch: " + name);
        }

        variable.value = value;
        return;
    }
    if (parent != nullptr) {
        parent->assign(name, value);
        return;
    }

    throw runtime_error("Undefined variable: " + name);
}

//registers function in current scope
void Environment::defineFunction(
    const string& name,
    const FunctionDeclaration* declaration
) {
    if (functions.contains(name)) {
        throw runtime_error("Function already defined: " + name);
    }

    functions[name] = {declaration, this};
}

//finds function in current or parent scope
FunctionBinding Environment::getFunction(const string& name) const {
    if (functions.contains(name)) {
        return functions.at(name);
    }
    if (parent != nullptr) {
        return parent->getFunction(name);
    }
    throw runtime_error("Undefined function: " + name);
}

//returns variables from current scope
const unordered_map<string, VariableInfo>& Environment::getVariables() const {
    return variables;
}