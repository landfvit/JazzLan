#include "Interpreter.hpp"

#include <iostream>
#include <stdexcept>

using namespace std;

//evaluates an expression and return its int value
int Interpreter::evaluateExpression(
    const Expression* expression,
    const Environment& environment
) {
    //Number: return its value
    if (
        const auto* number = dynamic_cast<const NumberExpression*>(expression)
    ) {
        return number->value;
    }

    //Identifier: find its current value in variables
    if (const auto* identifier =
        dynamic_cast<const IdentifierExpression*>(expression)) {
            return environment.get(identifier->name);
    }

    //Binary expression: eval both sides and apply operators
    if (
        const auto* binary = dynamic_cast<const BinaryExpression*>(expression)
    ) {
        int left = evaluateExpression(binary->left.get(), environment);
        int right = evaluateExpression(binary->right.get(), environment);

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

//executes the whole program in global scope
void Interpreter::executeProgram(const Program& program) {
    for (const auto& statement : program.statements) {
        executeStatement(statement.get(), global);
    }
}

//returns global variables
const unordered_map<string, int>& Interpreter::getVariables() const {
    return global.getVariables();
}

//executes a single statement in given scope
void Interpreter::executeStatement(
    const Statement* statement,
    Environment& environment
) {
    //variable declaration
    if (const auto* variable =
        dynamic_cast<const VariableDeclaration*>(statement)) {

        int value = evaluateExpression(variable->value.get(), environment);

        environment.define(variable->name, value);
        return;
    }
    //variable assignment
    if (const auto* assignment =
        dynamic_cast<const AssignmentStatement*>(statement)) {

        int value = evaluateExpression(assignment->value.get(), environment);

        environment.assign(assignment->name, value);
        return;
    }
    //print statement
    if (const auto* print =
        dynamic_cast<const PrintStatement*>(statement)) {

        int value = evaluateExpression(print->value.get(), environment);

        cout << value << endl;
        return;
    }
    //if/else statement
    if (const auto* ifStatement =
        dynamic_cast<const IfStatement*>(statement)) {

        int condition = evaluateExpression(
            ifStatement->condition.get(),
            environment
        );

        if (condition != 0) {
            executeBlock(ifStatement->thenBranch, environment);
        } else if (!ifStatement->elseBranch.empty()) {
            executeBlock(ifStatement->elseBranch, environment);
        }

        return;
    }
    //while loop
    if (const auto* loop =
        dynamic_cast<const WhileStatement*>(statement)) {

        while (evaluateExpression(loop->condition.get(), environment) != 0) {
            executeBlock(loop->body, environment);
        }

        return;
    }
    throw runtime_error("Unknown statement");
}

//executes statements in a new local scope
void Interpreter::executeBlock(
    const vector<unique_ptr<Statement>>& statements,
    Environment& parentEnvironment
) {
    Environment local(&parentEnvironment);

    for (const auto& statement : statements) {
        executeStatement(statement.get(), local);
    }
}