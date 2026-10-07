#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

//base type for all expressions
struct Expression {
    virtual ~Expression() = default;
};

//number expression
struct NumberExpression : Expression {
    int value;
};

//binary expression
struct BinaryExpression : Expression {
    std::unique_ptr<Expression> left;
    std::string op;
    std::unique_ptr<Expression> right;
};

//identifier expression
struct IdentifierExpression : Expression {
    std::string name;
};

//variable declaration
struct VariableDeclaration {
    std::string name;
    std::unique_ptr<Expression> value;
};

//whole program
struct Program {
    std::vector<VariableDeclaration> statements;
};

//debug printing
void printExpression(
    const Expression* expression,
    const std::unordered_map<std::string, int>& variables
);

void printVariableDeclaration(
    const VariableDeclaration& variable,
    const std::unordered_map<std::string, int>& variables
);

void printProgram(
    const Program& program,
    const std::unordered_map<std::string, int>& variables
);