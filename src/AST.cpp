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

//recursively prints statements and nested branches
void printStatementTree(
    const Statement* statement,
    const string& prefix,
    bool isLast,
    const unordered_map<string, int>& variables
) {
    cout << prefix << (isLast ? "└── " : "├── ");

    string childPrefix = prefix + (isLast ? "    " : "│   ");
    //variable declaration
    if (const auto* variable =
        dynamic_cast<const VariableDeclaration*>(statement)) {

        cout << "VariableDeclaration" << endl;
        cout << childPrefix << "├── name: " << variable->name << endl;
        cout << childPrefix << "└── value:" << endl;

        printExpressionTree(
            variable->value.get(),
            childPrefix + "    ",
            true,
            variables
        );
        return;
    }
    //print statement
    if (const auto* print =
        dynamic_cast<const PrintStatement*>(statement)) {

        cout << "PrintStatement" << endl;
        cout << childPrefix << "└── value:" << endl;

        printExpressionTree(
            print->value.get(),
            childPrefix + "    ",
            true,
            variables
        );

        return;
    }
    //if/else statement
    if (const auto* ifStatement =
        dynamic_cast<const IfStatement*>(statement)) {

        cout << "IfStatement" << endl;

        cout << childPrefix << "├── condition:" << endl;

        printExpressionTree(
            ifStatement->condition.get(),
            childPrefix + "│   ",
            true,
            variables
        );

        bool hasElse = !ifStatement->elseBranch.empty();

        cout << childPrefix
             << (hasElse ? "├── thenBranch:" : "└── thenBranch:")
             << endl;

        string thenPrefix = childPrefix + (hasElse ? "│   " : "    ");

        for (size_t i = 0; i < ifStatement->thenBranch.size(); i++) {
            printStatementTree(
                ifStatement->thenBranch[i].get(),
                thenPrefix,
                i + 1 == ifStatement->thenBranch.size(),
                variables
            );
        }
        if (hasElse) {
            cout << childPrefix << "└── elseBranch:" << endl;

            for (size_t i = 0; i < ifStatement->elseBranch.size(); i++) {
                printStatementTree(
                    ifStatement->elseBranch[i].get(),
                    childPrefix + "    ",
                    i + 1 == ifStatement->elseBranch.size(),
                    variables
                );
            }
        }
        return;
    }
    //variable assignment
    if (const auto* assignment =
        dynamic_cast<const AssignmentStatement*>(statement)) {

        cout << "AssignmentStatement" << endl;
        cout << childPrefix << "├── name: " << assignment->name << endl;
        cout << childPrefix << "└── value:" << endl;

        printExpressionTree(
            assignment->value.get(),
            childPrefix + "    ",
            true,
            variables
        );

        return;
    }
    cout << "UnknownStatement" << endl;
}

//prints the whole program AST
void printProgram(
    const Program& program,
    const unordered_map<string, int>& variables
) {
    cout << "Program" << endl;

    for (size_t i = 0; i < program.statements.size(); i++) {
        printStatementTree(
            program.statements[i].get(),
            "",
            i + 1 == program.statements.size(),
            variables
        );
    }
}