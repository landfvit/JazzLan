#include <iostream>
#include <string>
#include <vector>

#include "Lexer.hpp"
#include "Parser.hpp"
#include "AST.hpp"

using namespace std;

int main() {
    string source = "let x = y * (3 + 2);";

    vector<Token> tokens = tokenize(source);

    Parser parser(tokens);

    VariableDeclaration variable = parser.parseVariableDeclaration();

    printVariableDeclaration(variable);

    return 0;
}