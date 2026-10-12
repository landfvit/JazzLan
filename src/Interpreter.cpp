#include "Interpreter.hpp"

#include <iostream>
#include <stdexcept>

using namespace std;

//evaluates an expression and return its int value
int Interpreter::evaluateExpression(const Expression* expression) {
    //Number: return its value
    if (
        const auto* number = dynamic_cast<const NumberExpression*>(expression)
    ) {
        return number->value;
    }

    //Identifier: find its current value in variables
    if (
        const auto* identifier = dynamic_cast<const IdentifierExpression*>(expression)
    ) {
        if (!variables.contains(identifier->name)) {
            throw runtime_error("Undefined variable: " + identifier->name);
        }
        return variables.at(identifier->name);
    }

    //Binary expression: eval both sides and apply operators
    if (
        const auto* binary = dynamic_cast<const BinaryExpression*>(expression)
    ) {
        int left = evaluateExpression(binary->left.get());
        int right = evaluateExpression(binary->right.get());

        if (binary->op == "+") {
            return left + right;
        }

        if (binary->op == "-") {
            return left - right;
        }

        if (binary->op == "*") {
            return left * right;
        }

        if (binary->op == "/") {
            if (right == 0) {
                throw runtime_error("Division by zero");
            }
            return left / right;
        }

        //comparison operators
        if (binary->op == "<") {
            return left < right;
        }
        if (binary->op == "<=") {
            return left <= right;
        }
        if (binary->op == ">") {
            return left > right;
        }
        if (binary->op == ">=") {
            return left >= right;
        }

        //equality operators
        if (binary->op == "==") {
            return left == right;
        }
        if (binary->op == "!=") {
            return left != right;
        }

        throw runtime_error("Unknown operator: " + binary->op);
    }

    throw runtime_error("Unknown expression");
}

//executes the whole program
void Interpreter::executeProgram(const Program& program) {
    for (const auto& statement : program.statements) {
        executeStatement(statement.get());
    }
}

//provides variable values for AST Tree print
const unordered_map<string, int>& Interpreter::getVariables() const {
    return variables;
}

//executes a single statement
void Interpreter::executeStatement(const Statement* statement) {
    //variable declaration
    if (const auto* variable =
        dynamic_cast<const VariableDeclaration*>(statement)) {

        int value = evaluateExpression(variable->value.get());
        variables[variable->name] = value;

        return;
    }
    //print statement
    if (const auto* print =
        dynamic_cast<const PrintStatement*>(statement)) {

        int value = evaluateExpression(print->value.get());
        cout << value << endl;

        return;
    }
    //if/else statement
    if (const auto* ifStatement =
        dynamic_cast<const IfStatement*>(statement)) {

        int condition = evaluateExpression(ifStatement->condition.get());

        const auto& branch = condition != 0
            ? ifStatement->thenBranch
            : ifStatement->elseBranch;

        for (const auto& child : branch) {
            executeStatement(child.get());
        }
        return;
    }

    throw runtime_error("Unknown statement");
}