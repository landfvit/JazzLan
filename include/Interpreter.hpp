#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <unordered_map>

#include "AST.hpp"
#include "Environment.hpp"
#include "Value.hpp"

class Interpreter {
private:
    Environment global; //global scope

    Value evaluateExpression(
        const Expression* expression,
        const Environment& environment
    ); //evaluates expression

    std::optional<Value> callFunction(
        const CallExpression* call,
        const Environment& callerEnvironment
    ); //executes function call

    void executeStatement(
        const Statement* statement,
        Environment& environment
    ); //executes statement

    void executeBlock(
        const std::vector<std::unique_ptr<Statement>>& statements,
        Environment& parentEnvironment
    ); //executes block in local scope

public:
    void executeProgram(const Program& program); //executes program

    const std::unordered_map<std::string, VariableInfo>& getVariables() const;
};