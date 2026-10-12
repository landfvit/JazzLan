#include <iostream>

#include "Lexer.hpp"
#include "Parser.hpp"
#include "AST.hpp"
#include "Interpreter.hpp"

using namespace std;

int main() {
    string source = R"(
        let i = 0;

        while (i < 3) {
            let j = 0;

            while (j < 2) {
                print(i * 10 + j);
                j = j + 1;
            }

            i = i + 1;
        }
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