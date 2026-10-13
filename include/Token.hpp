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
    Void,
    In,
    Not,
    //values
    Identifier,
    Number,
    FloatNumber,
    String,
    True,
    False,
    None,
    Placeholder,
    //operators
    Plus,
    Minus,
    Star,
    Slash,
    Pipe,
    Ampersand,
    Caret,
    Equal,
    EqualEqual,
    Bang,
    BangEqual,
    Less,
    LessEqual,
    Greater,
    GreaterEqual,
    ListTransformStart,
    ListTransformEnd,
    SetTransformStart,
    SetTransformEnd,
    DictTransformStart,
    DictTransformEnd,
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
    Dot,
    
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
