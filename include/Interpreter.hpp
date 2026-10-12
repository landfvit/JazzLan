#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include "AST.hpp"
#include "Environment.hpp"

class Interpreter {
private:
    Environment global; //global scope

    int evaluateExpression(
        const Expression* expression,
        const Environment& environment
    ); //evaluates expression in given scope

    void executeStatement(
        const Statement* statement,
        Environment& environment
    ); //executes one statement

    void executeBlock(
        const std::vector<std::unique_ptr<Statement>>& statements,
        Environment& parentEnvironment
    ); //executes statements in a new local scope

    //registered functions awaiting call
    std::unordered_map<std::string, const FunctionDeclaration*> functions;

public:
    void executeProgram(const Program& program); //executes whole program

    const std::unordered_map<std::string, int>& getVariables() const; //returns global variables
};