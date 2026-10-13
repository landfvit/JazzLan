#pragma once

#include <string>
#include <unordered_map>

struct FunctionDeclaration;
class Environment;

//function definition and its scope
struct FunctionBinding {
    const FunctionDeclaration* declaration;
    Environment* definitionScope;
};

class Environment {
private:
    std::unordered_map<std::string, int> variables; //local variables
    std::unordered_map<std::string, FunctionBinding> functions; //local functions
    Environment* parent; //parent scope

public:
    Environment(Environment* parentEnviroment = nullptr);

    void define(const std::string& name, int value); //create 'x' in current scope
    int get(const std::string& name) const; //find 'x' in current or parent scope
    void assign(const std::string& name, int value); //changes 'x' value in nearest scope

    void defineFunction(
        const std::string& name,
        const FunctionDeclaration* declaration
    );

    FunctionBinding getFunction(const std::string& name) const;

    const std::unordered_map<std::string, int>& getVariables() const;
};