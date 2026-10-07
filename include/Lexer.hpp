#pragma once

#include <string>
#include <vector>
#include "Token.hpp"


std::vector<Token> tokenize(std::string source);
std::string tokenTypeToString(TokenType type);