#include "AST.hpp"
#include <iostream>

using namespace std;

//recursively prints one expression node as a tree
void printExpressionTree(
    const Expression* expression,
    const string& prefix,
    bool isLast,
    const unordered_map<string, int>& variables
) {
    cout << prefix;
    cout << (isLast ? "└── " : "├── ");

    //number node
    if (const auto* number =
        dynamic_cast<const NumberExpression*>(expression)) {

        cout << "NumberExpression(" << number->value << ")" << endl;
        return;
    }

    //identifier node
    if (const auto* identifier =
        dynamic_cast<const IdentifierExpression*>(expression)) {

        cout << "IdentifierExpression(" << identifier->name;

        if (variables.contains(identifier->name)) {
            cout << "=" << variables.at(identifier->name);
        }

        cout << ")" << endl;
        return;
    }

    //binary op. node
    if (const auto* binary =
        dynamic_cast<const BinaryExpression*>(expression)) {

        cout << "BinaryExpression(" << binary->op << ")" << endl;

        //keeps the tree lines aligned for child nodes
        string childPrefix =
            prefix + (isLast ? "    " : "│   ");

        printExpressionTree(
            binary->left.get(),
            childPrefix,
            false,
            variables
        );

        printExpressionTree(
            binary->right.get(),
            childPrefix,
            true,
            variables
        );

        return;
    }

    cout << "UnknownExpression" << endl;
}

//prints an expression from its root node
void printExpression(
    const Expression* expression,
    const unordered_map<string, int>& variables
) {
    printExpressionTree(
        expression,
        "",
        true,
        variables
    );
}

//prints one variable declaration and its expression tree
void printVariableDeclaration(
    const VariableDeclaration& variable,
    const unordered_map<string, int>& variables
) {
    cout << "VariableDeclaration" << endl;
    cout << "├── name: " << variable.name << endl;
    cout << "└── value:" << endl;

    printExpressionTree(
        variable.value.get(),
        "    ",
        true,
        variables
    );
}

//prints all statements in the program AST
void printProgram(
    const Program& program,
    const unordered_map<string, int>& variables
) {
    cout << "Program" << endl;

    for (const VariableDeclaration& statement : program.statements) {
        printVariableDeclaration(
            statement,
            variables
        );

        cout << endl;
    }
}