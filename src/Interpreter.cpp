#include "Interpreter.hpp"

#include <iostream>
#include <stdexcept>

using namespace std;

//signals a function return/fetch
struct ReturnSignal {
    int value;
};

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
    //function call expression
    if (const auto* call =
        dynamic_cast<const CallExpression*>(expression)) {
            optional<int> result = callFunction(call, environment);
            if (!result.has_value()) {
                throw runtime_error(
                    "FUnction returned no value: " + call->name
                );
            }
            return result.value();
        }

    throw runtime_error("Unknown expression");
}

//executes the whole program in global scope
void Interpreter::executeProgram(const Program& program) {
    try {
        for (const auto& statement : program.statements) {
            executeStatement(statement.get(), global);
        }
    }
    catch (const ReturnSignal&) {
        throw runtime_error("'return' or 'fetch' outside function");
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
    //function declaration
    if (const auto* function = 
        dynamic_cast<const FunctionDeclaration*>(statement)) {
            environment.defineFunction(function->name, function);
            return;
        }
    //return statement
    if (const auto* returnStatement =
        dynamic_cast<const ReturnStatement*>(statement)) {
            int value = evaluateExpression(
                returnStatement->value.get(),
                environment
            );
            throw ReturnSignal{value};
        }
    //standalone expression
    if (const auto& expressionStatement =
    dynamic_cast<const ExpressionStatement*>(statement)) {
        //function call without required return value
        if (const auto& call = 
        dynamic_cast<const CallExpression*>(expressionStatement->value.get())) {
            callFunction(call, environment);
        } else {
            evaluateExpression(expressionStatement->value.get(), environment);
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

//exectues function call
optional<int> Interpreter::callFunction(
    const CallExpression* call,
    const Environment& callerEnvironment
) {
    //find function in current or parent scope
    FunctionBinding binding = callerEnvironment.getFunction(call->name);
    const auto* function = binding.declaration;
    //cehck argument count
    if (call->arguments.size() != function->parameters.size()) {
        throw runtime_error("Invalid argument count: " + call->name);
    }
    //evaluate arguemtns in caller scope
    vector<int> arguments;
    for (const auto& argument : call->arguments) {
        arguments.push_back(
            evaluateExpression(argument.get(), callerEnvironment)
        );
    }
    //create function scope
    Environment local(binding.definitionScope);
    //bind arguments to parameters
    for (size_t i = 0; i < arguments.size(); i++) {
        local.define(function->parameters[i], arguments[i]);
    }
    //execute funtion body
    try {
        for (const auto& statement : function->body) {
            executeStatement(statement.get(), local);
        }
    }
    catch (const ReturnSignal& result) {
        return result.value;
    }
    //function finished without return
    return nullopt;
}