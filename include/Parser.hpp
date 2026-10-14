#pragma once

#include <vector>
#include <string>
#include <memory>

#include "Token.hpp"
#include "AST.hpp"


class Parser {
private:
    const std::vector<Token>& tokens; //reference to token seq produced by lexer
    size_t current = 0;               //current token index
    bool stopAtSetEnd = false;        //closing angle belongs to current set
    bool allowPlaceholder = false;   //current expression belongs to a transform
    size_t loopDepth = 0;            //loops inside the current function

    const Token& look();              //returns current token
    const Token& advance();           //returns token and moves forward
    bool check(TokenType type);       //checks current token type

    const Token& consume(
        TokenType type,
        const std::string& message
    );                                //expects token to have a given type or throws error

    std::unique_ptr<Expression> parsePrimary(); //values, literals and grouping
    std::unique_ptr<Expression> parsePostfix(); //indexing, slicing and methods
    std::unique_ptr<Expression> parseUnary(); //unary + and -
    std::unique_ptr<Expression> parseMult();    //* / - higher precedence
    std::unique_ptr<Expression> parseAdd();     //+ - - lower precedence
    std::unique_ptr<Expression> parseExpression(); //3 + 4
    std::unique_ptr<Expression> parseSetAnd(); //set intersection
    std::unique_ptr<Expression> parseSetXor(); //set symmetric difference
    std::unique_ptr<Expression> parseSetOr(); //set union
    std::unique_ptr<Expression> parseComparison(); //comparisons and membership
    std::unique_ptr<Expression> parseDelimitedExpression(bool setElement = false);
    std::vector<std::unique_ptr<Expression>> parseArguments(); //call arguments

    std::unique_ptr<Statement> parseStatement(); // print, if, while ...
    std::unique_ptr<Statement> parsePrintStatement(); //print specific
    std::vector<std::unique_ptr<Statement>> parseBlock(); //{....}
    std::unique_ptr<Statement> parseIfStatement(); //if/else ...
    std::unique_ptr<Statement> parseAssignmentStatement(); //assigns variable value
    std::unique_ptr<Statement> parseWhileStatement(); //while loop
    std::unique_ptr<Statement> parseForStatement(); //for loop
    std::unique_ptr<Statement> parseLoopControlStatement(); //break or continue
    std::unique_ptr<Statement> parseVariableDeclaration(); //let x = ...;
    std::unique_ptr<Statement> parseFunctionDeclaration(); //function definition
    std::unique_ptr<Statement> parseReturnStatement(); //return statement
    std::unique_ptr<Statement> parseExpressionStatement(); //standalone expression
    
    std::optional<ValueType> parseTypeAnnotation(bool allowVoid = false); //parses optional : type

public:
    Parser(const std::vector<Token>& sourceTokens); //creates parser

    Program parseProgram(); //parses whole program
};
