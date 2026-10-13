#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <exception>

#include "Lexer.hpp"
#include "Parser.hpp"
#include "AST.hpp"
#include "Interpreter.hpp"

using namespace std;

int main(int argc, char* argv[]) {
    //check command arguments
    if (argc < 2 || argc > 3) {
        cerr << "Usage: JazzLan <file.jazz> [--ast]" << endl;
        return 1;
    }

    //optional AST output
    bool showAST = false;

    if (argc == 3) {
        if (string(argv[2]) != "--ast") {
            cerr << "Unknown option: " << argv[2] << endl;
            return 1;
        }

        showAST = true;
    }

    //open source file
    ifstream file(argv[1]);

    if (!file.is_open()) {
        cerr << "Cannot open file: " << argv[1] << endl;
        return 1;
    }

    //read source code
    stringstream buffer;
    buffer << file.rdbuf();

    string source = buffer.str();

    try {
        //tokenize source code
        vector<Token> tokens = tokenize(source);

        //parse tokens into AST
        Parser parser(tokens);
        Program program = parser.parseProgram();

        //execute program
        Interpreter interpreter;
        interpreter.executeProgram(program);

        //optional AST debug output
        if (showAST) {
            cout << endl;
            printProgram(program, interpreter.getVariables());
        }
    }
    catch (const exception& error) {
        cerr << "JazzLanError: " << error.what() << endl;
        return 1;
    }

    return 0;
}