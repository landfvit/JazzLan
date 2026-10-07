#pragma once

#include <string>
#include <unordered_map>

#include "AST.hpp"

class Interpreter {
private:
    //stores variable names and their current int value
    std::unordered_map<std::string, int> variables;

    //evaluates an expression and returns its int result
    int evaluateExpression(const Expression* expression);

public:
    //executes all statements in program
    void executeProgram(const Program& program);

    //provides variable values for AST Tree print
    const std::unordered_map<std::string, int>& getVariables() const;
};