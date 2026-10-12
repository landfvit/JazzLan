#include <iostream>

#include "Lexer.hpp"
#include "Parser.hpp"
#include "AST.hpp"
#include "Interpreter.hpp"

using namespace std;

int main() {
    string source = R"(
        let x = 5;
        let y = 10;

        print(x < y);
        print(x >= y);
        print(x + 5 == y);
        print(x * 2 != y);
        print(3 + 2 * 4 > 10);
        print(5 > 3 == x < y);
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