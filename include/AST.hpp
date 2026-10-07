#pragma once
#include <string>

struct VariableDeclaration {
    std::string name;
    int value;
};

struct BinaryExpression {
    int left;
    std::string op;
    int right;
};


