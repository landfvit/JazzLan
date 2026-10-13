#include "Lexer.hpp"

#include <optional>

using namespace std;

//returns true if char is a digit from 0 to 9
bool isDigit(char c) {
    return '0' <= c && c <= '9';
}

//returns true if char is an uppercase or lowercase letter
bool isLetter(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

//returns true if char is allowed inside an identifier
//that includes digits, but not at the 0th index
//checked separately in tokenize()
bool isIdentifierChar(char c) {
    return isLetter(c) || isDigit(c) || c == '_';
}

//converts a complete word into its TokenType
//reserved keywords get their specific type
//other words are treated as identifier
TokenType getWordType(string word) {
    if (word == "let") {
        return TokenType::Let;
    }
    if (word == "print") {
        return TokenType::Print;
    }
    if (word == "if") {
        return TokenType::If;
    }
    if (word == "else") {
        return TokenType::Else;
    }
    if (word == "while") {
        return TokenType::While;
    }
    if (word == "function") {
        return TokenType::Function;
    }
    if (word == "return" || word == "fetch") {
        return TokenType::Return;
    }
    if (word == "void") {
        return TokenType::Void;
    }
    if (word == "in") {
        return TokenType::In;
    }
    if (word == "not") {
        return TokenType::Not;
    }
    if (word == "var") {
        return TokenType::Var;
    }
    if (word == "const") {
        return TokenType::Const;
    }
    if (word == "true") {
        return TokenType::True;
    }
    if (word == "false") {
        return TokenType::False;
    }
    if (word == "none") {
        return TokenType::None;
    }
    return TokenType::Identifier;
}

//converts source code into an ordered seq of tokens
vector<Token> tokenize(string source) {
    vector<Token> tokens;
    //scan source code from L to R, one char at a time
    for (size_t i = 0; i < source.size(); i++) {
        // ignore whitespace
        if (source[i] == ' ' || source[i] == '\n' || source[i] == '\t') {
            continue;
        }

        //-------------------------
        // keywords and identifiers
        //-------------------------

        //identifier must start witch letter or underscore
        if (isLetter(source[i]) || source[i] == '_') {
            string word;
            //collect entire identifier/keyword
            while (i < source.size() && isIdentifierChar(source[i])) { 
                word += source[i];
                i++; 
            }
            //decrease for the outer for loop when while loop condition is false to not skip
            i--;  
            tokens.push_back({getWordType(word), word});
            continue;
        }

        //--------
        // numbers
        //--------

        //numbers
        if (isDigit(source[i])) {
            string number;
            bool isFloat = false;

            while (i < source.size() && isDigit(source[i])) {
                number += source[i];
                i++;
            }

            //decimal part
            if (
                i + 1 < source.size() &&
                source[i] == '.' &&
                isDigit(source[i + 1])
            ) {
                isFloat = true;
                number += source[i];
                i++;

                while (i < source.size() && isDigit(source[i])) {
                    number += source[i];
                    i++;
                }
            }

            i--;

            tokens.push_back({
                isFloat ? TokenType::FloatNumber : TokenType::Number,
                number
            });

            continue;
        }

        //--------
        // strings
        //--------

        // " starts a string literal
        if (source[i] == '"') {
            string value;
            i++; //skips to look at char after "
            while (i < source.size() && source[i] != '"') { //continues until closing " is found
                value += source[i];
                i++;
            }
            tokens.push_back({TokenType::String, value});
            continue;
        }

        //----------
        // operators
        //----------

        //transform delimiters take priority over single symbols
        if (i + 1 < source.size()) {
            optional<TokenType> transform;

            if (source[i + 1] == '|') {
                switch (source[i]) {
                    case '[': transform = TokenType::ListTransformStart; break;
                    case '<': transform = TokenType::SetTransformStart; break;
                    case '{': transform = TokenType::DictTransformStart; break;
                }
            } else if (source[i] == '|') {
                switch (source[i + 1]) {
                    case ']': transform = TokenType::ListTransformEnd; break;
                    case '>': transform = TokenType::SetTransformEnd; break;
                    case '}': transform = TokenType::DictTransformEnd; break;
                }
            }

            if (transform.has_value()) {
                tokens.push_back({transform.value(), source.substr(i, 2)});
                i++;
                continue;
            }
        }

        //single char operators
        if (source[i] == '+') {
            tokens.push_back({TokenType::Plus, "+"});
        } else if (source[i] == '-') {
            tokens.push_back({TokenType::Minus, "-"});
        } else if (source[i] == '*') {
            tokens.push_back({TokenType::Star, "*"});
        } else if (source[i] == '/') {
            //skip line comments
            if (i + 1 < source.size() && source[i + 1] == '/') {
                while (i < source.size() && source[i] != '\n') {
                    i++;
                }
                continue;
            }
            tokens.push_back({TokenType::Slash, "/"});
        } else if (source[i] == '|') {
            tokens.push_back({TokenType::Pipe, "|"});
        } else if (source[i] == '&') {
            tokens.push_back({TokenType::Ampersand, "&"});
        } else if (source[i] == '^') {
            tokens.push_back({TokenType::Caret, "^"});

        //double char operators
        } else if (source[i] == '=') {
            if (i+1 < source.size() && source[i+1] == '=') {
                tokens.push_back({TokenType::EqualEqual, "=="});
                i++;
            } else {
                tokens.push_back({TokenType::Equal, "="});
            }
        } else if (source[i] == '!') {
            if (i+1 < source.size() && source[i+1] == '=') {
                tokens.push_back({TokenType::BangEqual, "!="});
                i++;
            } else {
                tokens.push_back({TokenType::Bang, "!"});
            }
        } else if (source[i] == '<') {
            if (i+1 < source.size() && source[i+1] == '=') {
                tokens.push_back({TokenType::LessEqual, "<="});
                i++;
            } else {
                tokens.push_back({TokenType::Less, "<"});
            }
        } else if (source[i] == '>') {
            //a set closing angle may be followed by equality
            if (
                i + 1 < source.size() && source[i + 1] == '=' &&
                !(i + 2 < source.size() && source[i + 2] == '=')
            ) {
                tokens.push_back({TokenType::GreaterEqual, ">="});
                i++;
            } else {
                tokens.push_back({TokenType::Greater, ">"});
            }
        }

        //--------
        // symbols
        //--------


        if (source[i] == '(') {
            tokens.push_back({TokenType::LeftParen, "("});
        } else if (source[i] == ')') {
            tokens.push_back({TokenType::RightParen, ")"});
        } else if (source[i] == '[') {
            tokens.push_back({TokenType::LeftBracket, "["});
        } else if (source[i] == ']') {
            tokens.push_back({TokenType::RightBracket, "]"});
        } else if (source[i] == '{') {
            tokens.push_back({TokenType::LeftBrace, "{"});
        } else if (source[i] == '}') {
            tokens.push_back({TokenType::RightBrace, "}"});
        } else if (source[i] == ',') {
            tokens.push_back({TokenType::Comma, ","});
        } else if (source[i] == ';') {
            tokens.push_back({TokenType::Semicolon, ";"});
        } else if (source[i] == ':') {
            tokens.push_back({TokenType::Colon, ":"});
        } else if (source[i] == '.') {
            tokens.push_back({TokenType::Dot, "."});
        } else if (source[i] == '#') {
            tokens.push_back({TokenType::Placeholder, "#"});
        }
    }

    // end of file
    tokens.push_back({TokenType::EndOfFile, ""});
    return tokens;
}

//converts TokenType into readable text - debug only
string tokenTypeToString(TokenType type) {
    switch (type) {
        //keywords
        case TokenType::Let: return "Let";
        case TokenType::Var: return "Var";
        case TokenType::Const: return "Const";
        case TokenType::Print: return "Print";
        case TokenType::If: return "If";
        case TokenType::Else: return "Else";
        case TokenType::While: return "While";
        case TokenType::Function: return "Function";
        case TokenType::Return: return "Return";
        case TokenType::Void: return "Void";
        case TokenType::In: return "In";
        case TokenType::Not: return "Not";
        //values
        case TokenType::Identifier: return "Identifier";
        case TokenType::Number: return "Number";
        case TokenType::FloatNumber: return "FloatNumber";
        case TokenType::String: return "String";
        case TokenType::True: return "True";
        case TokenType::False: return "False";
        case TokenType::None: return "None";
        case TokenType::Placeholder: return "Placeholder";
        //operators
        case TokenType::Plus: return "Plus";
        case TokenType::Minus: return "Minus";
        case TokenType::Star: return "Star";
        case TokenType::Slash: return "Slash";
        case TokenType::Pipe: return "Pipe";
        case TokenType::Ampersand: return "Ampersand";
        case TokenType::Caret: return "Caret";
        case TokenType::Equal: return "Equal";
        case TokenType::EqualEqual: return "EqualEqual";
        case TokenType::Bang: return "Bang";
        case TokenType::BangEqual: return "BangEqual";
        case TokenType::Less: return "Less";
        case TokenType::LessEqual: return "LessEqual";
        case TokenType::Greater: return "Greater";
        case TokenType::GreaterEqual: return "GreaterEqual";
        case TokenType::ListTransformStart: return "ListTransformStart";
        case TokenType::ListTransformEnd: return "ListTransformEnd";
        case TokenType::SetTransformStart: return "SetTransformStart";
        case TokenType::SetTransformEnd: return "SetTransformEnd";
        case TokenType::DictTransformStart: return "DictTransformStart";
        case TokenType::DictTransformEnd: return "DictTransformEnd";
        //symbols
        case TokenType::LeftParen: return "LeftParen";
        case TokenType::RightParen: return "RightParen";
        case TokenType::LeftBrace: return "LeftBrace";
        case TokenType::RightBrace: return "RightBrace";
        case TokenType::LeftBracket: return "LeftBracket";
        case TokenType::RightBracket: return "RightBracket";
        case TokenType::Comma: return "Comma";
        case TokenType::Semicolon: return "Semicolon";
        case TokenType::Colon: return "Colon";
        case TokenType::Dot: return "Dot";
        //EOF
        case TokenType::EndOfFile: return "EndOfFile";
    }
    return "Unknown";
}
