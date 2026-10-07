#include <iostream>
#include "Lexer.hpp"
#include "Parser.hpp"

using namespace std;

int main() {
    string source = "10 times 4";

vector<Token> tokens = tokenize(source);

Parser parser(tokens);

BinaryExpression expression = parser.parseBinaryExpression();

cout << expression.left << endl;
cout << expression.op << endl;
cout << expression.right << endl;
    return 0;
}