#pragma once

#include <string>
#include <optional>
#include <unordered_map>

#include "Value.hpp"
#include "VariableInfo.hpp"

struct FunctionDeclaration;
class Environment;

//function definition and its scope
struct FunctionBinding {
    const FunctionDeclaration* declaration;
    Environment* definitionScope;
};

class Environment {
private:
    std::unordered_map<std::string, VariableInfo> variables; //local variables
    std::unordered_map<std::string, FunctionBinding> functions; //local functions
    Environment* parent; //parent scope

public:
    Environment(Environment* parentEnvironment = nullptr);

    void define(
        const std::string& name,
        std::optional<Value> value,
        std::optional<ValueType> declaredType = std::nullopt,
        bool isConst = false
    );

    Value get(const std::string& name) const;
    void assign(const std::string& name, const Value& value);

    void defineFunction(
        const std::string& name,
        const FunctionDeclaration* declaration
    );

    FunctionBinding getFunction(const std::string& name) const;
    bool hasFunction(const std::string& name) const;

    const std::unordered_map<std::string, VariableInfo>& getVariables() const;
};
