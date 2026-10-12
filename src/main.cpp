#include <iostream>

#include "Lexer.hpp"
#include "Parser.hpp"
#include "AST.hpp"
#include "Interpreter.hpp"

using namespace std;

int main() {
    string source = R"(
        let x = 10;
        let x = 20;
    )";

    vector<Token> tokens = tokenize(source);

    Parser parser(tokens);
    Program program = parser.parseProgram();
    Interpreter interpreter;

    //show AST structure
    printProgram(
        program,
        interpreter.getVariables()
    );

    //execute the program
    interpreter.executeProgram(program);

    cout << endl;


    return 0;
}