#pragma once 

#include <string>

//all token categories lexer can currently recognise
enum class TokenType{
    //keywords
    Let,
    Print,
    If,
    Else,
    While,
    Function,
    Return,
    //values
    Identifier,
    Number,
    String,
    //operators
    Plus,
    Minus,
    Star,
    Slash,
    Equal,
    EqualEqual,
    Bang,
    BangEqual,
    Less,
    LessEqual,
    Greater,
    GreaterEqual,
    //symbols
    LeftParen,
    RightParen,
    LeftBracket,
    RightBracket,
    LeftBrace,
    RightBrace,
    Comma,
    Semicolon,
    
    //end of token stream
    EndOfFile
};

//structure of token produced by lexer
//'type' - from above
//'value' - original text from source code
struct Token{
    TokenType type;
    std::string value;
};