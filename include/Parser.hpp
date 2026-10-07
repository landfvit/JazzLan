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
    std::unique_ptr<Expression> parseExpression();

    VariableDeclaration parseVariableDeclaration(); //let x = ...;

public:
    Parser(const std::vector<Token>& sourceTokens); //creates parser

    Program parseProgram(); //parses whole program
};