#pragma once

#include <string>
#include <vector>
#include "Token.hpp"

//converts source code into an ordered seq of tokens
std::vector<Token> tokenize(std::string source); 

//converts TokenType enum value into readable text - debug only
std::string tokenTypeToString(TokenType type);