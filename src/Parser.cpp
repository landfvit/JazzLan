#include "Parser.hpp"

#include <stdexcept>
#include <limits>
#include <utility>
#include <unordered_set>

using namespace std;
 //init parses with source tokens
Parser::Parser(const vector<Token>& sourceTokens) 
    : tokens(sourceTokens) {
}

//returns current token wothout moving
const Token& Parser::look() { 
    if (current >= tokens.size()) {
        throw runtime_error("Unexpected end of input");
    }
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

//parses an expression inside its own delimiters
unique_ptr<Expression> Parser::parseDelimitedExpression(bool setElement) {
    bool previous = stopAtSetEnd;
    stopAtSetEnd = setElement;
    auto expression = parseExpression();
    stopAtSetEnd = previous;
    return expression;
}

//parses call arguments with an optional trailing comma
vector<unique_ptr<Expression>> Parser::parseArguments() {
    consume(TokenType::LeftParen, "Expected '('");
    vector<unique_ptr<Expression>> arguments;

    while (!check(TokenType::RightParen)) {
        arguments.push_back(parseDelimitedExpression());

        if (!check(TokenType::Comma)) {
            break;
        }
        advance();
    }

    consume(TokenType::RightParen, "Expected ')'");
    return arguments;
}

//parses values, literals, function calls and grouping
unique_ptr<Expression> Parser::parsePrimary() {
    //number expression
    if (check(TokenType::Number)) {
        const Token& numberToken = advance();

        auto expression = make_unique<NumberExpression>();
        expression->value = stoll(numberToken.value);

        return expression;
    }
    //float expression
    if (check(TokenType::FloatNumber)) {
        const Token& token = advance();

        auto expression = make_unique<FloatExpression>();
        expression->value = stod(token.value);

        return expression;
    }
    //string expression
    if (check(TokenType::String)) {
        const Token& token = advance();

        auto expression = make_unique<StringExpression>();
        expression->value = token.value;

        return expression;
    }
    //boolean expression
    if (check(TokenType::True) || check(TokenType::False)) {
        const Token& token = advance();

        auto expression = make_unique<BoolExpression>();
        expression->value = token.type == TokenType::True;

        return expression;
    }
    //none expression
    if (check(TokenType::None)) {
        advance();
        return make_unique<NoneExpression>();
    }
    //placeholder is available only in a transform expression
    if (check(TokenType::Placeholder)) {
        if (!allowPlaceholder) {
            throw runtime_error("Placeholder '#' is only allowed inside a transform");
        }
        advance();
        return make_unique<PlaceholderExpression>();
    }
    //list or set literal
    if (check(TokenType::LeftBracket) || check(TokenType::Less)) {
        bool isSet = check(TokenType::Less);
        advance();

        auto expression = make_unique<ContainerExpression>();
        expression->type = isSet ? ValueType::Set : ValueType::List;
        TokenType closing = isSet ? TokenType::Greater : TokenType::RightBracket;

        while (!check(closing)) {
            expression->elements.push_back(parseDelimitedExpression(isSet));

            if (!check(TokenType::Comma)) {
                break;
            }
            advance();
        }

        consume(closing, isSet ? "Expected '>'" : "Expected ']'");
        return expression;
    }
    //dictionary literal
    if (check(TokenType::LeftBrace)) {
        advance();
        auto expression = make_unique<DictionaryExpression>();

        while (!check(TokenType::RightBrace)) {
            auto key = parseDelimitedExpression();
            consume(TokenType::Colon, "Expected ':' between dictionary key and value");
            auto value = parseDelimitedExpression();
            expression->entries.emplace_back(std::move(key), std::move(value));

            if (!check(TokenType::Comma)) {
                break;
            }
            advance();
        }

        consume(TokenType::RightBrace, "Expected '}'");
        return expression;
    }
    //identifier or function call
    if (check(TokenType::Identifier)) {
        const Token& identifierToken = advance();
        //function call
        if (check(TokenType::LeftParen)) {
            auto expression = make_unique<CallExpression>();
            expression->name = identifierToken.value;
            expression->arguments = parseArguments();
            return expression;
        }
        //ordinary identifier
        auto expression = make_unique<IdentifierExpression>();
        expression->name = identifierToken.value;

        return expression;
    }
    //parenthesized expression
    if (check(TokenType::LeftParen)) {
        advance();
        auto expression = parseDelimitedExpression();
        consume(TokenType::RightParen, "Expected ')'");
        return expression;
    }
    throw runtime_error("Expected expression");
}

//parses chained indexing, slicing and method calls
unique_ptr<Expression> Parser::parsePostfix() {
    auto object = parsePrimary();

    while (check(TokenType::LeftBracket) || check(TokenType::Dot)) {
        if (check(TokenType::Dot)) {
            advance();
            const Token& name = consume(TokenType::Identifier, "Expected method name");
            auto call = make_unique<MethodCallExpression>();
            call->object = std::move(object);
            call->name = name.value;
            call->arguments = parseArguments();
            object = std::move(call);
            continue;
        }

        advance();
        unique_ptr<Expression> start;

        if (!check(TokenType::Colon)) {
            start = parseDelimitedExpression();
        }

        if (!check(TokenType::Colon)) {
            consume(TokenType::RightBracket, "Expected ']'");
            auto index = make_unique<IndexExpression>();
            index->object = std::move(object);
            index->index = std::move(start);
            object = std::move(index);
            continue;
        }

        advance();
        auto slice = make_unique<SliceExpression>();
        slice->object = std::move(object);
        slice->start = std::move(start);

        if (!check(TokenType::Colon) && !check(TokenType::RightBracket)) {
            slice->stop = parseDelimitedExpression();
        }
        if (check(TokenType::Colon)) {
            advance();

            if (!check(TokenType::RightBracket)) {
                slice->step = parseDelimitedExpression();
            }
        }

        consume(TokenType::RightBracket, "Expected ']'");
        object = std::move(slice);
    }

    return object;
}

//parses unary numeric operators
unique_ptr<Expression> Parser::parseUnary() {
    //the minimum integer has no positive int64 counterpart
    if (
        check(TokenType::Minus) &&
        current + 1 < tokens.size() &&
        tokens[current + 1].type == TokenType::Number &&
        stoll("-" + tokens[current + 1].value) == numeric_limits<int64_t>::min()
    ) {
        advance();
        advance();
        auto expression = make_unique<NumberExpression>();
        expression->value = numeric_limits<int64_t>::min();
        return expression;
    }

    if (check(TokenType::Plus) || check(TokenType::Minus)) {
        auto expression = make_unique<UnaryExpression>();
        expression->op = advance().value;
        expression->operand = parseUnary();
        return expression;
    }

    return parsePostfix();
}

//parser * and / before lower precedence operators
unique_ptr<Expression> Parser::parseMult() {
    auto left = parseUnary();

    while (
        check(TokenType::Star) ||
        check(TokenType::Slash)
    ) {
        const Token& operatorToken = advance();
        auto right = parseUnary();
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
    return parseComparison();
}

//parses variable declaration
unique_ptr<Statement> Parser::parseVariableDeclaration() {
    bool isConst = check(TokenType::Const);

    //declaration keyword
    if (
        check(TokenType::Var) ||
        check(TokenType::Const) ||
        check(TokenType::Let)
    ) {
        advance();
    } else {
        throw runtime_error("Expected variable declaration");
    }

    const Token& nameToken = consume(
        TokenType::Identifier,
        "Expected variable name"
    );

    //optional type
    auto declaredType = parseTypeAnnotation();

    //optional initializer
    unique_ptr<Expression> value;

    if (check(TokenType::Equal)) {
        advance();
        value = parseExpression();
    }

    consume(TokenType::Semicolon, "Expected ';'");

    //check declaration validity
    if (!declaredType.has_value() && !value) {
        throw runtime_error("Cannot infer type: " + nameToken.value);
    }
    if (isConst && !value) {
        throw runtime_error("Const requires initialization: " + nameToken.value);
    }

    auto statement = make_unique<VariableDeclaration>();

    statement->name = nameToken.value;
    statement->declaredType = declaredType;
    statement->isConst = isConst;
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
    if (
        check(TokenType::Var) ||
        check(TokenType::Const) ||
        check(TokenType::Let)
    ) {
        return parseVariableDeclaration();
    }
    if (check(TokenType::Print)) {
        return parsePrintStatement();
    }
    if (check(TokenType::If)) {
        return parseIfStatement();
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
    //simple variable assignment
    if (check(TokenType::Identifier)) {
        if (
            current + 1 < tokens.size() &&
            tokens[current + 1].type == TokenType::Equal
        ) {
            return parseAssignmentStatement();
        }
    }

    return parseExpressionStatement();
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

//parses set intersection
unique_ptr<Expression> Parser::parseSetAnd() {
    auto left = parseAdd();

    while (check(TokenType::Ampersand)) {
        auto expression = make_unique<BinaryExpression>();
        expression->left = std::move(left);
        expression->op = advance().value;
        expression->right = parseAdd();
        left = std::move(expression);
    }

    return left;
}

//parses set symmetric difference
unique_ptr<Expression> Parser::parseSetXor() {
    auto left = parseSetAnd();

    while (check(TokenType::Caret)) {
        auto expression = make_unique<BinaryExpression>();
        expression->left = std::move(left);
        expression->op = advance().value;
        expression->right = parseSetAnd();
        left = std::move(expression);
    }

    return left;
}

//parses set union
unique_ptr<Expression> Parser::parseSetOr() {
    auto left = parseSetXor();

    while (check(TokenType::Pipe)) {
        auto expression = make_unique<BinaryExpression>();
        expression->left = std::move(left);
        expression->op = advance().value;
        expression->right = parseSetXor();
        left = std::move(expression);
    }

    return left;
}

//parses comparisons and membership in one chain
unique_ptr<Expression> Parser::parseComparison() {
    auto chain = make_unique<ComparisonExpression>();
    chain->operands.push_back(parseSetOr());

    while (
        check(TokenType::Less) ||
        check(TokenType::LessEqual) ||
        (check(TokenType::Greater) && !stopAtSetEnd) ||
        check(TokenType::GreaterEqual) ||
        check(TokenType::EqualEqual) ||
        check(TokenType::BangEqual) ||
        check(TokenType::In) ||
        check(TokenType::Not)
    ) {
        string op = advance().value;

        if (op == "not") {
            consume(TokenType::In, "Expected 'in' after 'not'");
            op = "not in";
        }

        chain->operators.push_back(op);
        chain->operands.push_back(parseSetOr());
    }

    if (chain->operators.empty()) {
        return std::move(chain->operands[0]);
    }
    if (chain->operators.size() == 1) {
        auto expression = make_unique<BinaryExpression>();
        expression->left = std::move(chain->operands[0]);
        expression->op = chain->operators[0];
        expression->right = std::move(chain->operands[1]);
        return expression;
    }

    return chain;
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

    vector<FunctionParameter> parameters;
    unordered_set<string> usedParameters;
    //parse function parameters
    if (!check(TokenType::RightParen)) {
        do {
            const Token& parameter = consume(
                TokenType::Identifier,
                "Expected parameter name"
            );
            //check duplicate parameter
            if (!usedParameters.insert(parameter.value).second) {
                throw runtime_error(
                    "Duplicate parameter: " + parameter.value
                );
            }
            auto declaredType = parseTypeAnnotation();
            parameters.push_back({parameter.value, declaredType});
            if (!check(TokenType::Comma)) {
                break;
            }
            advance();
        } while (true);
    }

    consume(TokenType::RightParen, "Expected ')'");
    auto returnType = parseTypeAnnotation(true);
    auto body = parseBlock();
    auto statement = make_unique<FunctionDeclaration>();

    statement->name = nameToken.value;
    statement->parameters = std::move(parameters);
    statement->returnType = returnType;
    statement->body = std::move(body);

    return statement;
}

//handle return statement
unique_ptr<Statement> Parser::parseReturnStatement() {
    consume(TokenType::Return, "Expected 'return'");

    unique_ptr<Expression> value;

    if (!check(TokenType::Semicolon)) {
        value = parseExpression();
    }

    consume(TokenType::Semicolon, "Expected ';'");

    auto statement = make_unique<ReturnStatement>();
    statement->value = std::move(value);

    return statement;
}
//parses standalone expression
unique_ptr<Statement> Parser::parseExpressionStatement() {
    auto value = parseExpression();

    //transform into the same container or a new variable
    if (
        check(TokenType::ListTransformStart) ||
        check(TokenType::SetTransformStart) ||
        check(TokenType::DictTransformStart)
    ) {
        auto statement = make_unique<TransformStatement>();
        TokenType closing;
        string closingValue;

        switch (advance().type) {
            case TokenType::ListTransformStart:
                statement->type = ValueType::List;
                closing = TokenType::ListTransformEnd;
                closingValue = "|]";
                break;
            case TokenType::SetTransformStart:
                statement->type = ValueType::Set;
                closing = TokenType::SetTransformEnd;
                closingValue = "|>";
                break;
            case TokenType::DictTransformStart:
                statement->type = ValueType::Dict;
                closing = TokenType::DictTransformEnd;
                closingValue = "|}";
                break;
            default:
                throw runtime_error("Expected transform delimiter");
        }

        bool previous = allowPlaceholder;
        allowPlaceholder = true;
        auto expression = parseExpression();
        allowPlaceholder = previous;
        consume(closing, "Expected '" + closingValue + "' after transform expression");

        statement->source = std::move(value);
        statement->expression = std::move(expression);

        if (check(TokenType::Identifier)) {
            statement->target = advance().value;
        }

        consume(TokenType::Semicolon, "Expected ';'");
        return statement;
    }

    //assignment to an indexed element
    if (check(TokenType::Equal)) {
        advance();
        auto assignedValue = parseExpression();
        consume(TokenType::Semicolon, "Expected ';'");

        if (auto* index = dynamic_cast<IndexExpression*>(value.get())) {
            auto statement = make_unique<IndexAssignmentStatement>();
            statement->object = std::move(index->object);
            statement->index = std::move(index->index);
            statement->value = std::move(assignedValue);
            return statement;
        }
        if (const auto* identifier = dynamic_cast<const IdentifierExpression*>(value.get())) {
            auto statement = make_unique<AssignmentStatement>();
            statement->name = identifier->name;
            statement->value = std::move(assignedValue);
            return statement;
        }

        throw runtime_error("Invalid assignment target");
    }

    consume(TokenType::Semicolon, "Expected ';'");

    auto statement = make_unique<ExpressionStatement>();
    statement->value = std::move(value);

    return statement;
}

//parses optional type annotation
optional<ValueType> Parser::parseTypeAnnotation(bool allowVoid) {
    if (!check(TokenType::Colon)) {
        return nullopt;
    }

    advance();

    if (check(TokenType::None)) {
        advance();
        return ValueType::None;
    }
    if (check(TokenType::Void)) {
        if (!allowVoid) {
            throw runtime_error("Void is only allowed as a function return type");
        }
        advance();
        return ValueType::Void;
    }

    const Token& typeToken = consume(
        TokenType::Identifier,
        "Expected type name"
    );

    if (typeToken.value == "int") {
        return ValueType::Int;
    }
    if (typeToken.value == "float") {
        return ValueType::Float;
    }
    if (typeToken.value == "string") {
        return ValueType::String;
    }
    if (typeToken.value == "bool") {
        return ValueType::Bool;
    }
    if (typeToken.value == "list") {
        return ValueType::List;
    }
    if (typeToken.value == "set") {
        return ValueType::Set;
    }
    if (typeToken.value == "dict") {
        return ValueType::Dict;
    }

    throw runtime_error("Unknown type: " + typeToken.value);
}
