#include <iostream>
#include "Lexer.hpp"

using namespace std;

void printTokens(const vector<Token>& tokens){
    for (const Token& token : tokens){
        cout << tokenTypeToString(token.type) << " -> " << token.value << endl;
    }
}

int main() {
    string source = R"(
        let score = 15;

        if (score >= 10) {
            print("great");
        }

        let values = [1, 2, 3];
    )";

    vector<Token> tokens = tokenize(source);

    cout << "Token count: " << tokens.size() << endl;

    printTokens(tokens);

    return 0;
}