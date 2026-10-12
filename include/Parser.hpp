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

    const Token& look();              //returns current token
    const Token& advance();           //returns token and moves forward
    bool check(TokenType type);       //checks current token type

    const Token& consume(
        TokenType type,
        const std::string& message
    );                                //expects token to have a given type or throws error

    std::unique_ptr<Expression> parsePrimary(); //numbers, identifiers, ()
    std::unique_ptr<Expression> parseMult();    //* / - higher precedence
    std::unique_ptr<Expression> parseAdd();     //+ - - lower precedence
    std::unique_ptr<Expression> parseExpression(); //3 + 4
    std::unique_ptr<Expression> parseComparison(); //< > <= >= ...
    std::unique_ptr<Expression> parseEquality(); //== !=

    std::unique_ptr<Statement> parseStatement(); // print, if, while ...
    std::unique_ptr<Statement> parsePrintStatement(); //print specific
    std::vector<std::unique_ptr<Statement>> parseBlock(); //{....}
    std::unique_ptr<Statement> parseIfStatement(); //if/else ...
    std::unique_ptr<Statement> parseAssignmentStatement(); //assigns variable value
    std::unique_ptr<Statement> parseWhileStatement(); //while loop
    std::unique_ptr<Statement> parseVariableDeclaration(); //let x = ...;
    std::unique_ptr<Statement> parseFunctionDeclaration(); //function definition

public:
    Parser(const std::vector<Token>& sourceTokens); //creates parser

    Program parseProgram(); //parses whole program
};