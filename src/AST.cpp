#include "AST.hpp"
#include <iostream>

using namespace std;

void printExpressionTree(
    const Expression* expression,
    const string& prefix,
    bool isLast
) {
    cout << prefix;
    cout << (isLast ? "└── " : "├── ");

    if (const auto* number =
        dynamic_cast<const NumberExpression*>(expression)) {

        cout << "NumberExpression(" << number->value << ")" << endl;
        return;
    }

    if (const auto* identifier =
        dynamic_cast<const IdentifierExpression*>(expression)) {

        cout << "IdentifierExpression(" << identifier->name << ")" << endl;
        return;
    }

    if (const auto* binary =
        dynamic_cast<const BinaryExpression*>(expression)) {

        cout << "BinaryExpression(" << binary->op << ")" << endl;

        string childPrefix =
            prefix + (isLast ? "    " : "│   ");

        printExpressionTree(
            binary->left.get(),
            childPrefix,
            false
        );

        printExpressionTree(
            binary->right.get(),
            childPrefix,
            true
        );

        return;
    }

    cout << "UnknownExpression" << endl;
}


void printExpression(const Expression* expression) {
    if (const auto* binary =
        dynamic_cast<const BinaryExpression*>(expression)) {

        cout << "BinaryExpression(" << binary->op << ")" << endl;

        printExpressionTree(
            binary->left.get(),
            "",
            false
        );

        printExpressionTree(
            binary->right.get(),
            "",
            true
        );

        return;
    }

    printExpressionTree(
        expression,
        "",
        true
    );
}


void printVariableDeclaration(
    const VariableDeclaration& variable
) {
    cout << "VariableDeclaration" << endl;
    cout << "├── name: " << variable.name << endl;
    cout << "└── value:" << endl;

    printExpressionTree(
        variable.value.get(),
        "    ",
        true
    );
}