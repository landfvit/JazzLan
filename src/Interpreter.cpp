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

        throw runtime_error("Unknown operator: " + binary->op);
    }

    throw runtime_error("Unknown expression");
}

//executes all variable declarations in the program
void Interpreter::executeProgram(const Program& program) {
    for (const VariableDeclaration& statement : program.statements) {
        int value = evaluateExpression(statement.value.get());
        variables[statement.name] = value;
        cout << statement.name << " = " << value << endl;
    }
}

//provides variable values for AST Tree print
const unordered_map<string, int>& Interpreter::getVariables() const {
    return variables;
}