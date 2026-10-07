#include "Parser.hpp"

#include <stdexcept>

using namespace std;

Parser::Parser(const vector<Token>& sourceTokens) 
    : tokens(sourceTokens) {
}

const Token& Parser::look() { 
    return tokens[current];
}

const Token& Parser::advance() {
    const Token& currentToken = look();
    current++;
    return currentToken;
}

bool Parser::check(TokenType type) {
    return look().type == type;
}

const Token& Parser::consume(TokenType type, const string& message) {
    if (check(type)) {
        return advance();
    }
    throw runtime_error(message);
}

VariableDeclaration Parser::parseVariableDeclaration() {
    consume(TokenType::Let, "Expected 'let'");

    const Token& nameToken = consume(
        TokenType::Identifier,
        "Expected variable name"
    );

    consume(TokenType::Equal, "Expected '='");

    const Token& valueToken = consume(
        TokenType::Number,
        "Expected number"
    );

    consume(TokenType::Semicolon, "Expected ';'");

    int value = stoi(valueToken.value);

    return {
        nameToken.value,
        value
    };
}

BinaryExpression Parser::parseBinaryExpression() {
    const Token& leftToken = consume(
        TokenType::Number,
        "Expected number"
    );

    if (
        !check(TokenType::Plus) &&
        !check(TokenType::Minus) &&
        !check(TokenType::Star) &&
        !check(TokenType::Slash)
    ) {
        throw runtime_error("Expected operator");
    }

    const Token& operatorToken = advance();

    const Token& rightToken = consume(
        TokenType::Number,
        "Expected number"
    );

    int leftValue = stoi(leftToken.value);
    int rightValue = stoi(rightToken.value);

    return {
        leftValue,
        operatorToken.value,
        rightValue
    };
}

