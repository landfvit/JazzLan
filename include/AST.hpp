#pragma once

#include <optional>
#include "Value.hpp"
#include "VariableInfo.hpp"

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <utility>

//base type for all expressions
struct Expression {
    virtual ~Expression() = default;
};

//integer expression
struct NumberExpression : Expression {
    std::int64_t value;
};

//float expression
struct FloatExpression : Expression {
    double value;
};

//string expression
struct StringExpression : Expression {
    std::string value;
};

//boolean expression
struct BoolExpression : Expression {
    bool value;
};

//none expression
struct NoneExpression : Expression {
};

//current element inside a transform
struct PlaceholderExpression : Expression {
};

//list or set literal
struct ContainerExpression : Expression {
    ValueType type;
    std::vector<std::unique_ptr<Expression>> elements;
};

//dictionary literal
struct DictionaryExpression : Expression {
    std::vector<std::pair<
        std::unique_ptr<Expression>,
        std::unique_ptr<Expression>
    >> entries;
};

//unary expression
struct UnaryExpression : Expression {
    std::string op;
    std::unique_ptr<Expression> operand;
};

//binary expression
struct BinaryExpression : Expression {
    std::unique_ptr<Expression> left;
    std::string op;
    std::unique_ptr<Expression> right;
};

//chained comparison
struct ComparisonExpression : Expression {
    std::vector<std::unique_ptr<Expression>> operands;
    std::vector<std::string> operators;
};

//identifier expression
struct IdentifierExpression : Expression {
    std::string name;
};

//function call
struct CallExpression : Expression {
    std::string name;
    std::vector<std::unique_ptr<Expression>> arguments;
};

//container element access
struct IndexExpression : Expression {
    std::unique_ptr<Expression> object;
    std::unique_ptr<Expression> index;
};

//sequence slice with optional bounds
struct SliceExpression : Expression {
    std::unique_ptr<Expression> object;
    std::unique_ptr<Expression> start;
    std::unique_ptr<Expression> stop;
    std::unique_ptr<Expression> step;
};

//method call on a value
struct MethodCallExpression : Expression {
    std::unique_ptr<Expression> object;
    std::string name;
    std::vector<std::unique_ptr<Expression>> arguments;
};

//base type for statements
struct Statement {
    virtual ~Statement() = default;
};

//variable declaration
struct VariableDeclaration : Statement {
    std::string name;
    std::optional<ValueType> declaredType;
    bool isConst = false;
    std::unique_ptr<Expression> value;
};

//print(...)
struct PrintStatement : Statement {
    std::unique_ptr<Expression> value;
};

//if/else statement
struct IfStatement : Statement {
    std::unique_ptr<Expression> condition;
    std::vector<std::unique_ptr<Statement>> thenBranch;
    std::vector<std::unique_ptr<Statement>> elseBranch;
};

//variable assignment
struct AssignmentStatement : Statement {
    std::string name;
    std::unique_ptr<Expression> value;
};

//container element assignment
struct IndexAssignmentStatement : Statement {
    std::unique_ptr<Expression> object;
    std::unique_ptr<Expression> index;
    std::unique_ptr<Expression> value;
};

//transform elements in place or into a new variable
struct TransformStatement : Statement {
    ValueType type;
    std::unique_ptr<Expression> source;
    std::unique_ptr<Expression> expression;
    std::optional<std::string> target;
};

//while statement
struct WhileStatement : Statement {
    std::unique_ptr<Expression> condition;
    std::vector<std::unique_ptr<Statement>> body;
};

//function parameter with optional type
struct FunctionParameter {
    std::string name;
    std::optional<ValueType> declaredType;
};

//function declaration
struct FunctionDeclaration : Statement {
    std::string name;
    std::vector<FunctionParameter> parameters;
    std::optional<ValueType> returnType;
    std::vector<std::unique_ptr<Statement>> body;
};

//returns from function with optional value
struct ReturnStatement : Statement {
    std::unique_ptr<Expression> value;
};

struct ExpressionStatement : Statement {
    std::unique_ptr<Expression> value;
};

//whole program
struct Program {
    std::vector<std::unique_ptr<Statement>> statements;
};

//debug printing
void printExpression(
    const Expression* expression,
    const std::unordered_map<std::string, VariableInfo>& variables
);

void printVariableDeclaration(
    const VariableDeclaration& variable,
    const std::unordered_map<std::string, VariableInfo>& variables
);

void printProgram(
    const Program& program,
    const std::unordered_map<std::string, VariableInfo>& variables
);
