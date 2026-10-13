#pragma once 

#include <string>

//all token categories lexer can currently recognise
enum class TokenType{
    //keywords
    Let,
    Var,
    Const,
    Print,
    If,
    Else,
    While,
    Function,
    Return,
    //values
    Identifier,
    Number,
    FloatNumber,
    String,
    True,
    False,
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
    Colon,
    
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