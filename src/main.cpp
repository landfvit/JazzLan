#include <iostream>

#include "Lexer.hpp"
#include "Parser.hpp"
#include "AST.hpp"
#include "Interpreter.hpp"

using namespace std;

int main() {
    string source = R"(
        let x = 5;
        let y = x + 3;
        let z = y * (x + 2);
    )";

    vector<Token> tokens = tokenize(source);

    Parser parser(tokens);
    Program program = parser.parseProgram();

    //execute the program
    Interpreter interpreter;
    interpreter.executeProgram(program);

    cout << endl;

    //show AST structure
    printProgram(
        program,
        interpreter.getVariables()
    );

    return 0;
}