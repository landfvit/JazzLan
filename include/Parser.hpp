#pragma once

#include <vector>
#include <string>

#include "Token.hpp"
#include "AST.hpp"

class Parser {
private:
    const std::vector<Token>& tokens; //reference to tokens from source code
    size_t current = 0; //index of curren token

    const Token& look(); //returns current token and does not move
    const Token& advance(); //return current token and moves forward
    bool check(TokenType type); //checks if current token has given type

    const Token& consume(
        TokenType type,
        const std::string& message
    ); // returns expected token or throws an error

public:
    Parser(const std::vector<Token>& sourceTokens); //creates parser from source tokens
    VariableDeclaration parseVariableDeclaration(); //parses stuff like: let <identifier> = <number>;
    BinaryExpression parseBinaryExpression(); //parses stuff like: 5 + 3
};