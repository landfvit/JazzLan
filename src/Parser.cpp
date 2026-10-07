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

//----------------------------------------------------//
//|||||||||||||PARSER FUNCTIONS|||||||||||||||||||||||//
//VVVVVVVVVVVVVXXXXXXXXXXXXXXXXVVVVVVVVVVVVVVVVVVVVVVV//

unique_ptr<Expression> Parser::parsePrimary() {
    if (check(TokenType::Number)) {
        const Token& numberToken = advance();
        auto expression = make_unique<NumberExpression>();
        expression->value = stoi(numberToken.value);

        return expression;
    }
    if (check(TokenType::LeftParen)) {
        advance();
        auto expression = parseExpression();
        consume(
            TokenType::RightParen,
            "Expected ')'"
        );

        return expression;
    }

    throw runtime_error("Expected expression");
}

unique_ptr<Expression> Parser::parseMult() {
    auto left = parsePrimary();

    while (
        check(TokenType::Star) ||
        check(TokenType::Slash)
    ) {
        const Token& operatorToken = advance();
        auto right = parsePrimary();
        auto expression = make_unique<BinaryExpression>();

        expression->left = std::move(left);
        expression->op = operatorToken.value;
        expression->right = std::move(right);

        left = std::move(expression);
    }

    return left;
}

unique_ptr<Expression> Parser::parseAdd() {
    auto left = parseMult();

    while (
        check(TokenType::Plus) ||
        check(TokenType::Minus)
    ) {
        const Token& operatorToken = advance();
        auto right = parseMult();
        auto expression = make_unique<BinaryExpression>();

        expression->left = std::move(left);
        expression->op = operatorToken.value;
        expression->right = std::move(right);

        left = std::move(expression);
    }

    return left;
}

unique_ptr<Expression> Parser::parseExpression() {
    return parseAdd();
}

VariableDeclaration Parser::parseVariableDeclaration() {
    consume(TokenType::Let, "Expected 'let'");

    const Token& nameToken = consume(
        TokenType::Identifier,
        "Expected variable name"
    );

    consume(TokenType::Equal, "Expected '='");

    unique_ptr<Expression> value = parseExpression();

    consume(TokenType::Semicolon, "Expected ';'");

    return {
        nameToken.value,
        std::move(value)
    };
}

