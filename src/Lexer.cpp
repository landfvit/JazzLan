#include "Lexer.hpp"

using namespace std;

bool isDigit(char c){
    return '0' <= c && c <= '9';
}

bool isLetter(char c){
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

bool isIdentifierChar(char c){
    return isLetter(c) || isDigit(c) || c == '_';
}

TokenType getWordType(string word){
    /*
    Helper function, asigns types to keyword tokens. If no match is found, the Identifier type is used.
    */
    if (word == "let"){
        return TokenType::Let;
    }
    if (word == "print"){
        return TokenType::Print;
    }
    if (word == "if"){
        return TokenType::If;
    }
    if (word == "else"){
        return TokenType::Else;
    }
    if (word == "while"){
        return TokenType::While;
    }
    if (word == "function"){
        return TokenType::Function;
    }
    if (word == "return"){
        return TokenType::Return;
    }
    return TokenType::Identifier;
}

vector<Token> tokenize(string source){
    /*
    Assigns tokens to an input sequence of chars.
    */
    vector<Token> tokens;
    for (size_t i = 0; i < source.size(); i++){
        // ignore whitespace
        if (source[i] == ' ' || source[i] == '\n' || source[i] == '\t'){
            continue;
        }

        // keywords and identifiers
        if (isLetter(source[i]) || source[i] == '_'){
            string word;
            while (i < source.size() && isIdentifierChar(source[i])){ //continue while the char is valid inside an identifier
                word += source[i];
                i++; //increase for the inner while loop
            }
            i--; //decrease for the outer for loop when while loop condition is false to not skip 
            tokens.push_back({getWordType(word), word});
            continue;
        }

        // numbers
        if (isDigit(source[i])){
            string number;
            while (i < source.size() && isDigit(source[i])){ //check for digits
                number += source[i];
                i++;
            }
            i--;
            tokens.push_back({TokenType::Number, number});
            continue;
        }

        // strings
        if (source[i] == '"'){ //checks for opening "
            string value;
            i++; //skips to look at char after "
            while (i < source.size() && source[i] != '"'){ //continues until closing " is found
                value += source[i];
                i++;
            }
            tokens.push_back({TokenType::String, value});
            continue;
        }

        // operators
        if (source[i] == '+'){
            tokens.push_back({TokenType::Plus, "+"});
        } else if (source[i] == '-'){
            tokens.push_back({TokenType::Minus, "-"});
        } else if (source[i] == '*'){
            tokens.push_back({TokenType::Star, "*"});
        } else if (source[i] == '/'){
            tokens.push_back({TokenType::Slash, "/"});
        } else if (source[i] == '='){
            if (i+1 < source.size() && source[i+1] == '='){
                tokens.push_back({TokenType::EqualEqual, "=="});
                i++;
            } else {
                tokens.push_back({TokenType::Equal, "="});
            }
        } else if (source[i] == '!'){
            if (i+1 < source.size() && source[i+1] == '='){
                tokens.push_back({TokenType::BangEqual, "!="});
                i++;
            } else {
                tokens.push_back({TokenType::Bang, "!"});
            }
        } else if (source[i] == '<'){
            if (i+1 < source.size() && source[i+1] == '='){
                tokens.push_back({TokenType::LessEqual, "<="});
                i++;
            } else {
                tokens.push_back({TokenType::Less, "<"});
            }
        } else if (source[i] == '>'){
            if (i+1 < source.size() && source[i+1] == '='){
                tokens.push_back({TokenType::GreaterEqual, ">="});
                i++;
            } else {
                tokens.push_back({TokenType::Greater, ">"});
            }
        }

        // symbols
        if (source[i] == '('){
            tokens.push_back({TokenType::LeftParen, "("});
        } else if (source[i] == ')'){
            tokens.push_back({TokenType::RightParen, ")"});
        } else if (source[i] == '['){
            tokens.push_back({TokenType::LeftBracket, "["});
        } else if (source[i] == ']'){
            tokens.push_back({TokenType::RightBracket, "]"});
        } else if (source[i] == '{'){
            tokens.push_back({TokenType::LeftBrace, "{"});
        } else if (source[i] == '}'){
            tokens.push_back({TokenType::RightBrace, "}"});
        } else if (source[i] == ','){
            tokens.push_back({TokenType::Comma, ","});
        } else if (source[i] == ';'){
            tokens.push_back({TokenType::Semicolon, ";"});
        }
    }
    // end of file
    tokens.push_back({TokenType::EndOfFile, ""});
    return tokens;
}

string tokenTypeToString(TokenType type){
    /*
    made for Lexer testing
    */
    switch (type){
        //keywords
        case TokenType::Let: return "Let";
        case TokenType::Print: return "Print";
        case TokenType::If: return "If";
        case TokenType::Else: return "Else";
        case TokenType::While: return "While";
        case TokenType::Function: return "Function";
        case TokenType::Return: return "Return";
        //values
        case TokenType::Identifier: return "Identifier";
        case TokenType::Number: return "Number";
        case TokenType::String: return "String";
        //operators
        case TokenType::Plus: return "Plus";
        case TokenType::Minus: return "Minus";
        case TokenType::Star: return "Star";
        case TokenType::Slash: return "Slash";
        case TokenType::Equal: return "Equal";
        case TokenType::EqualEqual: return "EqualEqual";
        case TokenType::Bang: return "Bang";
        case TokenType::BangEqual: return "BangEqual";
        case TokenType::Less: return "Less";
        case TokenType::LessEqual: return "LessEqual";
        case TokenType::Greater: return "Greater";
        case TokenType::GreaterEqual: return "GreaterEqual";
        //symbols
        case TokenType::LeftParen: return "LeftParen";
        case TokenType::RightParen: return "RightParen";
        case TokenType::LeftBrace: return "LeftBrace";
        case TokenType::RightBrace: return "RightBrace";
        case TokenType::LeftBracket: return "LeftBracket";
        case TokenType::RightBracket: return "RightBracket";
        case TokenType::Comma: return "Comma";
        case TokenType::Semicolon: return "Semicolon";
        //EOF
        case TokenType::EndOfFile: return "EndOfFile";
    }
    return "Unknown";
}

