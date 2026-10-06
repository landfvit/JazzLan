#pragma once //read once
#include <string>
using namespace std;


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
    //special
    EndOfFile
};

struct Token{
    TokenType type;
    string value;
};