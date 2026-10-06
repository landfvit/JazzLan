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
        let x = 5;
        let player_name1 = 42;
        let _score2026 = 10;
        print("ahoj");
    )";

    vector<Token> tokens = tokenize(source);

    cout << "Token count: " << tokens.size() << endl;

    printTokens(tokens);

    return 0;
}