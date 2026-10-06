#include "Lexer.hpp"

using namespace std;

bool isDigit(char c){
    return '0' <= c && c <= '9';
}

bool isLetter(char c){
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
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
        //ignore whitespace
        if (source[i] == ' ' || source[i] == '\n' || source[i] == '\t'){
            continue;
        }

        //words and keywords
        if (isLetter(source[i])){
            string word;
            while (i < source.size() && isLetter(source[i])){ //checks for letters
                word += source[i];
                i++; //increase for the inner while loop
            }
            i--; //decrease for the outer for loop when while loop condition is false to not skip 
            tokens.push_back({getWordType(word), word});
            continue;
        }

        //numbers
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

        //

    }

}