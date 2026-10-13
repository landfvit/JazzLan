#include "Interpreter.hpp"

#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <utility>

using namespace std;

//signals function return/fetch
struct ReturnSignal {
    optional<Value> value;
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
    ValueType type = getValueType(value);
    if (type == ValueType::List || type == ValueType::Set || type == ValueType::Dict) {
        return containerSize(value) != 0;
    }
    throw runtime_error("Condition must be bool or number");
}

//evaluates one operator using shared container equality
Value Interpreter::evaluateBinary(const Value& left, const string& op, const Value& right) {
    if (op == "==" || op == "!=") {
        bool equal = valuesEqual(left, right);
        return op == "==" ? equal : !equal;
    }
    if (op == "in" || op == "not in") {
        bool present = containsValue(right, left);
        return op == "in" ? present : !present;
    }
    if (auto result = containerOperation(left, op, right, containers)) {
        return result.value();
    }
    if (op == "<" || op == "<=" || op == ">" || op == ">=") {
        ValueType leftType = getValueType(left);
        ValueType rightType = getValueType(right);
        bool numeric = (isNumeric(left) || leftType == ValueType::Bool) &&
                       (isNumeric(right) || rightType == ValueType::Bool);
        bool sequence = leftType == rightType &&
                        (leftType == ValueType::String || leftType == ValueType::List);
        if (!numeric && !sequence) {
            throw runtime_error("Comparison requires numeric operands");
        }
        if (op == "<") return valueLess(left, right);
        if (op == ">") return valueLess(right, left);
        if (op == "<=") return valueLess(left, right) || valuesEqual(left, right);
        return valueLess(right, left) || valuesEqual(left, right);
    }
    bool bothIntegers =
        holds_alternative<int64_t>(left) &&
        holds_alternative<int64_t>(right);

    bool bothNumeric = isNumeric(left) && isNumeric(right);

    //addition and string concatenation
    if (op == "+") {
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
        op == "-" ||
        op == "*" ||
        op == "/"
    ) {
        if (!bothNumeric) {
            throw runtime_error(
                "Operator requires numeric operands: " + op
            );
        }
        if (op == "-") {
            if (bothIntegers) {
                return get<int64_t>(left) - get<int64_t>(right);
            }
            return asDouble(left) - asDouble(right);
        }
        if (op == "*") {
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
    throw runtime_error("Unknown operator: " + op);
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
    //none expression
    if (dynamic_cast<const NoneExpression*>(expression)) {
        return monostate{};
    }
    //current element inside a transform
    if (dynamic_cast<const PlaceholderExpression*>(expression)) {
        return environment.get("#");
    }
    //identifier expression
    if (const auto* identifier =
        dynamic_cast<const IdentifierExpression*>(expression)) {
        return environment.get(identifier->name);
    }
    //container literal
    if (const auto* literal = dynamic_cast<const ContainerExpression*>(expression)) {
        Value result = literal->type == ValueType::List
            ? Value{containers.create<ListValue>()}
            : Value{containers.create<SetValue>()};
        for (const auto& element : literal->elements) {
            Value value = evaluateExpression(element.get(), environment);
            callContainerMethod(result, literal->type == ValueType::List ? "append" : "add", {value}, containers);
        }
        return result;
    }
    //dictionary literal
    if (const auto* literal = dynamic_cast<const DictionaryExpression*>(expression)) {
        Value dictionary = containers.create<DictValue>();
        for (const auto& entry : literal->entries) {
            Value key = evaluateExpression(entry.first.get(), environment);
            Value value = evaluateExpression(entry.second.get(), environment);
            setItem(dictionary, key, value);
        }
        return dictionary;
    }
    //unary number expression
    if (const auto* unary = dynamic_cast<const UnaryExpression*>(expression)) {
        Value value = evaluateExpression(unary->operand.get(), environment);
        if (holds_alternative<bool>(value)) {
            value = int64_t{get<bool>(value) ? 1 : 0};
        }
        if (!isNumeric(value)) {
            throw runtime_error("Unary operator requires a number");
        }
        if (unary->op == "+") return value;
        if (holds_alternative<double>(value)) return -get<double>(value);
        int64_t integer = get<int64_t>(value);
        if (integer == numeric_limits<int64_t>::min()) {
            throw runtime_error("Integer overflow");
        }
        return -integer;
    }
    //binary expression
    if (const auto* binary = dynamic_cast<const BinaryExpression*>(expression)) {
        Value left = evaluateExpression(binary->left.get(), environment);
        Value right = evaluateExpression(binary->right.get(), environment);
        return evaluateBinary(left, binary->op, right);
    }
    //chained comparisons evaluate each operand at most once
    if (const auto* comparison = dynamic_cast<const ComparisonExpression*>(expression)) {
        Value left = evaluateExpression(comparison->operands[0].get(), environment);
        for (size_t i = 0; i < comparison->operators.size(); i++) {
            Value right = evaluateExpression(comparison->operands[i + 1].get(), environment);
            if (!get<bool>(evaluateBinary(left, comparison->operators[i], right))) {
                return false;
            }
            left = std::move(right);
        }
        return true;
    }
    //indexed container access
    if (const auto* index = dynamic_cast<const IndexExpression*>(expression)) {
        Value object = evaluateExpression(index->object.get(), environment);
        Value position = evaluateExpression(index->index.get(), environment);
        return getItem(object, position);
    }
    //sequence slice
    if (const auto* slice = dynamic_cast<const SliceExpression*>(expression)) {
        Value object = evaluateExpression(slice->object.get(), environment);
        optional<Value> start;
        optional<Value> stop;
        optional<Value> step;
        if (slice->start) start = evaluateExpression(slice->start.get(), environment);
        if (slice->stop) stop = evaluateExpression(slice->stop.get(), environment);
        if (slice->step) step = evaluateExpression(slice->step.get(), environment);
        return sliceValue(object, start, stop, step, containers);
    }
    //container method call
    if (const auto* call = dynamic_cast<const MethodCallExpression*>(expression)) {
        Value object = evaluateExpression(call->object.get(), environment);
        vector<Value> arguments;
        for (const auto& argument : call->arguments) {
            arguments.push_back(evaluateExpression(argument.get(), environment));
        }
        return callContainerMethod(object, call->name, arguments, containers);
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
    //container transformation with an optional new variable
    if (const auto* statementTransform = dynamic_cast<const TransformStatement*>(statement)) {
        if (
            statementTransform->target.has_value() &&
            environment.getVariables().contains(statementTransform->target.value())
        ) {
            throw runtime_error("Variable already defined: " + statementTransform->target.value());
        }

        Value source = evaluateExpression(statementTransform->source.get(), environment);
        ValueType type = getValueType(source);
        if (type != ValueType::List && type != ValueType::Set && type != ValueType::Dict) {
            throw runtime_error("Transform requires a container");
        }
        if (type != statementTransform->type) {
            throw runtime_error(
                "Transform expects " + valueTypeToString(statementTransform->type) +
                ", got " + valueTypeToString(type)
            );
        }
        if (!statementTransform->target.has_value()) {
            requireMutable(source);
        }

        Value result = transformContainer(source, [&](const Value& element) {
            Environment local(&environment);
            local.define("#", element);
            return evaluateExpression(statementTransform->expression.get(), local);
        }, containers);

        if (statementTransform->target.has_value()) {
            environment.define(statementTransform->target.value(), std::move(result));
        } else {
            replaceContainerContents(source, result);
        }
        return;
    }
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
    //indexed assignment evaluates the new value before its target
    if (const auto* assignment = dynamic_cast<const IndexAssignmentStatement*>(statement)) {
        Value value = evaluateExpression(assignment->value.get(), environment);
        Value object = evaluateExpression(assignment->object.get(), environment);
        Value index = evaluateExpression(assignment->index.get(), environment);
        setItem(object, index, value);
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

        optional<Value> value;

        if (returnStatement->value) {
            value = evaluateExpression(
                returnStatement->value.get(),
                environment
            );
        }

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
    //user functions may shadow container constructors
    bool builtin = call->name == "list" || call->name == "set" ||
                   call->name == "dict" || call->name == "len";
    if (builtin && !callerEnvironment.hasFunction(call->name)) {
        vector<Value> arguments;
        for (const auto& argument : call->arguments) {
            arguments.push_back(evaluateExpression(argument.get(), callerEnvironment));
        }
        return callContainerFunction(call->name, arguments, containers);
    }

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
        const auto& parameter = function->parameters[i];

        if (
            parameter.declaredType.has_value() &&
            getValueType(arguments[i]) != parameter.declaredType.value()
        ) {
            throw runtime_error(
                "Argument type mismatch: " + call->name +
                " (parameter " + parameter.name +
                ", expected " + valueTypeToString(parameter.declaredType.value()) +
                ", got " + valueTypeToString(getValueType(arguments[i])) + ")"
            );
        }

        local.define(parameter.name, arguments[i], parameter.declaredType);
    }

    //execute function body
    optional<Value> result;

    try {
        for (const auto& statement : function->body) {
            executeStatement(statement.get(), local);
        }
    }
    catch (const ReturnSignal& signal) {
        result = signal.value;
    }

    //check optional return type
    if (function->returnType.has_value()) {
        ValueType returnType = function->returnType.value();

        if (returnType == ValueType::Void) {
            if (result.has_value()) {
                throw runtime_error(
                    "Void function cannot return a value: " + call->name
                );
            }
        } else {
            if (!result.has_value()) {
                throw runtime_error(
                    "Function must return " + valueTypeToString(returnType) +
                    ": " + call->name
                );
            }
            if (getValueType(result.value()) != returnType) {
                throw runtime_error(
                    "Return type mismatch: " + call->name +
                    " (expected " + valueTypeToString(returnType) +
                    ", got " + valueTypeToString(getValueType(result.value())) + ")"
                );
            }
        }
    }

    return result;
}
