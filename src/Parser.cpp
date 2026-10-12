#include "Parser.hpp"

#include <stdexcept>
#include <utility>

using namespace std;
 //init parses with source tokens
Parser::Parser(const vector<Token>& sourceTokens) 
    : tokens(sourceTokens) {
}

//returns current token wothout moving
const Token& Parser::look() { 
    return tokens[current];
}

//returns current token and moves
const Token& Parser::advance() {
    const Token& currentToken = look();
    current++;
    return currentToken;
}

//checks if current token has given type
bool Parser::check(TokenType type) {
    return look().type == type;
}

//consumes expected token or throws error
const Token& Parser::consume(TokenType type, const string& message) {
    if (check(type)) {
        return advance();
    }
    throw runtime_error(message);
}

//----------------
//parser functions
//----------------

//parses numbers, identifiers and paren. expressions
unique_ptr<Expression> Parser::parsePrimary() {
    if (check(TokenType::Number)) {
        const Token& numberToken = advance();

        auto expression = make_unique<NumberExpression>();
        expression->value = stoi(numberToken.value);

        return expression;
    }

    if (check(TokenType::Identifier)) {
        const Token& identifierToken = advance();

        auto expression = make_unique<IdentifierExpression>();
        expression->name = identifierToken.value;

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

//parser * and / before lower precedence operators
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

//parses + and - usinf mult expressions as operands
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

//entry point for expression parsing -> lowest precedence operation
unique_ptr<Expression> Parser::parseExpression() {
    return parseEquality();
}

//parser: let <identifier> = <expression>;
unique_ptr<Statement> Parser::parseVariableDeclaration() {
    consume(TokenType::Let, "Expected 'let'");

    const Token& nameToken = consume(
        TokenType:: Identifier,
        "Expected variable name"
    );
    consume(TokenType::Equal, "Expected '='");

    auto value = parseExpression();

    consume(TokenType::Semicolon, "Expected ';'");

    auto statement = make_unique<VariableDeclaration>();

    statement->name = nameToken.value;
    statement->value = std::move(value);

    return statement;
}

//parses statements unitl EOF
Program Parser::parseProgram() {
    Program program;

    while (!check(TokenType::EndOfFile)) {
        program.statements.push_back(
            parseStatement()
        );
    }

    return program;
}

//selects correct statement parser
unique_ptr<Statement> Parser::parseStatement() {
    if (check(TokenType::Let)) {
        return parseVariableDeclaration();
    }
    if (check(TokenType::Print)) {
        return parsePrintStatement();
    }
    if (check(TokenType::If)) {
        return parseIfStatement();
    }
    if (check(TokenType::Identifier)) {
        return parseAssignmentStatement();
    }
    if (check(TokenType::While)) {
        return parseWhileStatement();
    }
    if (check(TokenType::Function)) {
        return parseFunctionDeclaration();
    }
    if (check(TokenType::Return)) {
        return parseReturnStatement();
    }

    throw runtime_error("Expected statement");
}

//parser Print(...)
unique_ptr<Statement> Parser::parsePrintStatement() {
    consume(TokenType::Print, "Expected 'Print'");
    consume(TokenType::LeftParen, "Expected '('");

    auto value = parseExpression();

    consume(TokenType::RightParen, "Expected ')'");
    consume(TokenType::Semicolon, "Expected ';'");

    auto statement = make_unique<PrintStatement>();

    statement->value = std::move(value);

    return statement;
}

//parses comparison operators
unique_ptr<Expression> Parser::parseComparison() {
    auto left = parseAdd();

    while (
        check(TokenType::Less) ||
        check(TokenType::LessEqual) ||
        check(TokenType::Greater) ||
        check(TokenType::GreaterEqual) 
    ) {
        const Token& operatorToken = advance();
        auto right = parseAdd();

        auto expression = make_unique<BinaryExpression>();

        expression->left = std::move(left);
        expression->op = operatorToken.value;
        expression->right = std::move(right);

        left = std::move(expression);
    }

    return left;
}

//parser == and != operators
unique_ptr<Expression> Parser::parseEquality() {
    auto left = parseComparison();

    while (
        check(TokenType::EqualEqual) ||
        check(TokenType::BangEqual)
    ) {
        const Token& operatorToken = advance();
        auto right = parseComparison();

        auto expression = make_unique<BinaryExpression>();

        expression->left = std::move(left);
        expression->op = operatorToken.value;
        expression->right = std::move(right);

        left = std::move(expression);
    }

    return left;
}

//parses a block of statement
vector<unique_ptr<Statement>> Parser::parseBlock() {
    consume(TokenType::LeftBrace, "Expected '{'");
    
    vector<unique_ptr<Statement>> statements;

    while (
        !check(TokenType::RightBrace) &&
        !check(TokenType::EndOfFile)
    ) {
        statements.push_back(parseStatement());
    }

    consume(TokenType::RightBrace, "Expected '}'");

    return statements;
}

//parses if/else statement
unique_ptr<Statement> Parser::parseIfStatement() {
    consume(TokenType::If, "Expected 'if'");
    consume(TokenType::LeftParen, "Expected '('");

    auto condition = parseExpression();

    consume(TokenType::RightParen, "Expected ')'");

    auto thenBranch = parseBlock();
    auto statement = make_unique<IfStatement>();

    statement->condition = std::move(condition);
    statement->thenBranch = std::move(thenBranch);

    //optional else branch
    if (check(TokenType::Else)) {
        advance();

        auto elseBranch = parseBlock();
        statement->elseBranch = std::move(elseBranch);
    }

    return statement;
}

//parses variable assignment
unique_ptr<Statement> Parser::parseAssignmentStatement() {
    const Token& nameToken = consume(
        TokenType::Identifier,
        "Expected variable name"
    );

    consume(TokenType::Equal, "Expected '='");

    auto value = parseExpression();

    consume(TokenType::Semicolon, "Expected ';'");

    auto statement = make_unique<AssignmentStatement>();

    statement->name = nameToken.value;
    statement->value = std::move(value);

    return statement;
}

//while loop
unique_ptr<Statement> Parser::parseWhileStatement() {
    consume(TokenType::While, "Expected 'while'");
    consume(TokenType::LeftParen, "Expected '('");

    auto condition = parseExpression();

    consume(TokenType::RightParen, "Expected ')'");

    auto body = parseBlock();

    auto statement = make_unique<WhileStatement>();

    statement->condition = std::move(condition);
    statement->body = std::move(body);

    return statement;
}

//function declaration
unique_ptr<Statement> Parser::parseFunctionDeclaration() {
    consume(TokenType::Function, "Expected 'function'");

    const Token& nameToken = consume(
        TokenType::Identifier,
        "expected function name"
    );

    consume(TokenType::LeftParen, "Expected '('");

    vector<string> parameters;

    //parse function parameters
    if (!check(TokenType::RightParen)) {
        do {
            const Token& parameter = consume(
                TokenType::Identifier,
                "Expected parameter name"
            );
            parameters.push_back(parameter.value);
            if (!check(TokenType::Comma)) {
                break;
            }

            advance();
        } while (true);
    }
    consume(TokenType::RightParen, "Expected ')'");
    auto body = parseBlock();
    auto statement = make_unique<FunctionDeclaration>();

    statement->name = nameToken.value;
    statement->parameters = std::move(parameters);
    statement->body = std::move(body);

    return statement;
}

//handle return statement
unique_ptr<Statement> Parser::parseReturnStatement() {
    consume(TokenType::Return, "Expected 'return'");

    auto value = parseExpression();

    consume(TokenType::Semicolon, "Expected ';'");

    auto statement = make_unique<ReturnStatement>();
    statement->value = std::move(value);

    return statement;
}

