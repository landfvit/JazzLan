#include <iostream>
#include <exception>

#include "Lexer.hpp"
#include "Parser.hpp"
#include "AST.hpp"
#include "Interpreter.hpp"

using namespace std;

int main() {
    string source = R"(
        function add(a, b) {
            fetch a + b;
        }

        function maximum(a, b) {
            if (a > b) {
                fetch a;
            }

            return b;
        }

        let x = add(5, 3);

        print(x);
        print(maximum(10, 20));
    )";

    //tokenize source code
    vector<Token> tokens = tokenize(source);

    //parse tokens into AST
    Parser parser(tokens);
    Program program = parser.parseProgram();

    Interpreter interpreter;

    //execute program
    try {
        interpreter.executeProgram(program);
    }
    catch (const exception& error) {
        cerr << "RuntimeError: " << error.what() << endl;
        return 1;
    }

    cout << endl;

    //show AST structure
    printProgram(
        program,
        interpreter.getVariables()
    );

    return 0;
}