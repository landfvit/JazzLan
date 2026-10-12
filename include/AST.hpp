#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

//base type for all expressions
struct Expression {
    virtual ~Expression() = default;
};

//number expression
struct NumberExpression : Expression {
    int value;
};

//binary expression
struct BinaryExpression : Expression {
    std::unique_ptr<Expression> left;
    std::string op;
    std::unique_ptr<Expression> right;
};

//identifier expression
struct IdentifierExpression : Expression {
    std::string name;
};

//base type for statements
struct Statement {
    virtual ~Statement() = default;
};

//variable declaration
struct VariableDeclaration : Statement {
    std::string name;
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

//while statement
struct WhileStatement : Statement {
    std::unique_ptr<Expression> condition;
    std::vector<std::unique_ptr<Statement>> body;
};

//whole program
struct Program {
    std::vector<std::unique_ptr<Statement>> statements;
};

//debug printing
void printExpression(
    const Expression* expression,
    const std::unordered_map<std::string, int>& variables
);

void printVariableDeclaration(
    const VariableDeclaration& variable,
    const std::unordered_map<std::string, int>& variables
);

void printProgram(
    const Program& program,
    const std::unordered_map<std::string, int>& variables
);