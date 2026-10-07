#pragma once

#include <memory>
#include <string>

struct Expression {
    virtual ~Expression() = default;
};

struct NumberExpression : Expression {
    int value;
};

struct BinaryExpression : Expression {
    std::unique_ptr<Expression> left;
    std::string op;
    std::unique_ptr<Expression> right;
};

struct VariableDeclaration {
    std::string name;
    std::unique_ptr<Expression> value;
};

void printExpression(const Expression* expression);
void printVariableDeclaration(const VariableDeclaration& variable);

