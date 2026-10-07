#include "AST.hpp"
#include <iostream>

using namespace std;

void printExpression(const Expression* expression) {
    if (const auto* number = dynamic_cast<const NumberExpression*>(expression)) {
        cout << "Number expression(" << number->value << ")";
        return;
    }
    if (const auto* binary = dynamic_cast<const BinaryExpression*>(expression)) {
        cout << "BinaryExpression(";

        printExpression(binary->left.get());

        cout << " " << binary->op << " ";

        printExpression(binary->right.get());

        cout << ")";
    }
}

void printVariableDeclaration(const VariableDeclaration& variable) {
    cout << "VariableDeclaration" << endl;
    cout << "name: " << variable.name << endl;
    cout << "value: ";

    printExpression(variable.value.get());

    cout << endl;
}