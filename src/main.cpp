#include <iostream>

#include "Lexer.hpp"
#include "Parser.hpp"
#include "AST.hpp"
#include "Interpreter.hpp"

using namespace std;

int main() {
    string source = R"(
        let x = 10;
        let y = 20;

        if (x < y) {
            print(x);

            if (x == 10) {
                print(100);
            }
        } else {
            print(y);
        }
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