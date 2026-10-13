#include "Interpreter.hpp"

#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <utility>

using namespace std;

//signals function return/fetch
struct ReturnSignal {
    Value value;
};
//checks numeric values
static bool isNumeric(const Value& value) {
    return holds_alternative<int64_t>(value) || holds_alternative<double>(value);
}
//converts numeric value to double
static double asDouble(const Value& value) {
    if (holds_alternative<int64_t>(value)) {
        return static_cast<double>(get<int64_t>(value));
    }
    if (holds_alternative<double>(value)) {
        return get<double>(value);
    }
    throw runtime_error("Expected numeric value");
}
//evaluates conditions
static bool isTruthy(const Value& value) {
    if (holds_alternative<bool>(value)) {
        return get<bool>(value);
    }
    if (holds_alternative<int64_t>(value)) {
        return get<int64_t>(value) != 0;
    }
    if (holds_alternative<double>(value)) {
        return get<double>(value) != 0.0;
    }
    throw runtime_error("Condition must be bool or number");
}

//evaluates expression in given scope
Value Interpreter::evaluateExpression(
    const Expression* expression,
    const Environment& environment
) {
    if (expression == nullptr) {
        throw runtime_error("Missing expression");
    }
    //integer expression
    if (const auto* number =
        dynamic_cast<const NumberExpression*>(expression)) {
        return number->value;
    }
    //float expression
    if (const auto* number =
        dynamic_cast<const FloatExpression*>(expression)) {
        return number->value;
    }
    //string expression
    if (const auto* text =
        dynamic_cast<const StringExpression*>(expression)) {
        return text->value;
    }
    //boolean expression
    if (const auto* boolean =
        dynamic_cast<const BoolExpression*>(expression)) {
        return boolean->value;
    }
    //identifier expression
    if (const auto* identifier =
        dynamic_cast<const IdentifierExpression*>(expression)) {
        return environment.get(identifier->name);
    }
    //binary expression
    if (const auto* binary =
        dynamic_cast<const BinaryExpression*>(expression)) {

        Value left = evaluateExpression(binary->left.get(), environment);
        Value right = evaluateExpression(binary->right.get(), environment);

        bool bothIntegers =
            holds_alternative<int64_t>(left) &&
            holds_alternative<int64_t>(right);

        bool bothNumeric = isNumeric(left) && isNumeric(right);

        //addition and string concatenation
        if (binary->op == "+") {
            if (
                holds_alternative<string>(left) &&
                holds_alternative<string>(right)
            ) {
                return get<string>(left) + get<string>(right);
            }
            if (!bothNumeric) {
                throw runtime_error(
                    "Operator '+' requires two numbers or two strings"
                );
            }
            if (bothIntegers) {
                return get<int64_t>(left) + get<int64_t>(right);
            }
            return asDouble(left) + asDouble(right);
        }
        //arithmetic operators
        if (
            binary->op == "-" ||
            binary->op == "*" ||
            binary->op == "/"
        ) {
            if (!bothNumeric) {
                throw runtime_error(
                    "Operator requires numeric operands: " + binary->op
                );
            }
            if (binary->op == "-") {
                if (bothIntegers) {
                    return get<int64_t>(left) - get<int64_t>(right);
                }
                return asDouble(left) - asDouble(right);
            }
            if (binary->op == "*") {
                if (bothIntegers) {
                    return get<int64_t>(left) * get<int64_t>(right);
                }
                return asDouble(left) * asDouble(right);
            }
            if (asDouble(right) == 0.0) {
                throw runtime_error("Division by zero");
            }
            if (bothIntegers) {
                int64_t a = get<int64_t>(left);
                int64_t b = get<int64_t>(right);

                if (a == numeric_limits<int64_t>::min() && b == -1) {
                    throw runtime_error("Integer overflow");
                }
                return a / b;
            }
            return asDouble(left) / asDouble(right);
        }
        //comparison operators
        if (
            binary->op == "<" ||
            binary->op == "<=" ||
            binary->op == ">" ||
            binary->op == ">="
        ) {
            if (!bothNumeric) {
                throw runtime_error("Comparison requires numeric operands");
            }
            if (bothIntegers) {
                int64_t a = get<int64_t>(left);
                int64_t b = get<int64_t>(right);

                if (binary->op == "<") return a < b;
                if (binary->op == "<=") return a <= b;
                if (binary->op == ">") return a > b;
                return a >= b;
            }
            double a = asDouble(left);
            double b = asDouble(right);

            if (binary->op == "<") return a < b;
            if (binary->op == "<=") return a <= b;
            if (binary->op == ">") return a > b;
            return a >= b;
        }
        //equality operators
        if (binary->op == "==" || binary->op == "!=") {
            bool equal;

            if (bothNumeric) {
                if (bothIntegers) {
                    equal = get<int64_t>(left) == get<int64_t>(right);
                } else {
                    equal = asDouble(left) == asDouble(right);
                }
            } else {
                equal = left == right;
            }

            return binary->op == "==" ? equal : !equal;
        }
        throw runtime_error("Unknown operator: " + binary->op);
    }
    //function call expression
    if (const auto* call =
        dynamic_cast<const CallExpression*>(expression)) {

        optional<Value> result = callFunction(call, environment);

        if (!result.has_value()) {
            throw runtime_error(
                "Function returned no value: " + call->name
            );
        }
        return result.value();
    }
    throw runtime_error("Unknown expression");
}

//executes whole program in global scope
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
const unordered_map<string, VariableInfo>& Interpreter::getVariables() const {
    return global.getVariables();
}

//executes single statement in given scope
void Interpreter::executeStatement(
    const Statement* statement,
    Environment& environment
) {
    //variable declaration
    if (const auto* variable =
        dynamic_cast<const VariableDeclaration*>(statement)) {

        optional<Value> value;

        if (variable->value) {
            value = evaluateExpression(variable->value.get(), environment);
        }

        environment.define(
            variable->name,
            std::move(value),
            variable->declaredType,
            variable->isConst
        );
        return;
    }
    //variable assignment
    if (const auto* assignment =
        dynamic_cast<const AssignmentStatement*>(statement)) {

        Value value = evaluateExpression(
            assignment->value.get(),
            environment
        );

        environment.assign(assignment->name, value);
        return;
    }
    //print statement
    if (const auto* print =
        dynamic_cast<const PrintStatement*>(statement)) {

        Value value = evaluateExpression(
            print->value.get(),
            environment
        );

        cout << valueToString(value) << endl;
        return;
    }
    //if/else statement
    if (const auto* ifStatement =
        dynamic_cast<const IfStatement*>(statement)) {

        Value condition = evaluateExpression(
            ifStatement->condition.get(),
            environment
        );

        if (isTruthy(condition)) {
            executeBlock(ifStatement->thenBranch, environment);
        } else if (!ifStatement->elseBranch.empty()) {
            executeBlock(ifStatement->elseBranch, environment);
        }
        return;
    }
    //while loop
    if (const auto* loop =
        dynamic_cast<const WhileStatement*>(statement)) {

        while (isTruthy(
            evaluateExpression(loop->condition.get(), environment)
        )) {
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

        Value value = evaluateExpression(
            returnStatement->value.get(),
            environment
        );

        throw ReturnSignal{std::move(value)};
    }
    //standalone expression
    if (const auto* expressionStatement =
        dynamic_cast<const ExpressionStatement*>(statement)) {

        if (const auto* call =
            dynamic_cast<const CallExpression*>(expressionStatement->value.get())) {

            callFunction(call, environment);
        } else {
            evaluateExpression(expressionStatement->value.get(), environment);
        }
        return;
    }
    throw runtime_error("Unknown statement");
}

//executes statements in new local scope
void Interpreter::executeBlock(
    const vector<unique_ptr<Statement>>& statements,
    Environment& parentEnvironment
) {
    Environment local(&parentEnvironment);

    for (const auto& statement : statements) {
        executeStatement(statement.get(), local);
    }
}

//executes function call
optional<Value> Interpreter::callFunction(
    const CallExpression* call,
    const Environment& callerEnvironment
) {
    //find function in current or parent scope
    FunctionBinding binding = callerEnvironment.getFunction(call->name);

    const auto* function = binding.declaration;

    //check argument count
    if (call->arguments.size() != function->parameters.size()) {
        throw runtime_error("Invalid argument count: " + call->name);
    }

    //evaluate arguments in caller scope
    vector<Value> arguments;

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

    //execute function body
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