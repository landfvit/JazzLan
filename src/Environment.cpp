#include "Environment.hpp"

#include <stdexcept>

using namespace std;

//init environment with optioanl parent scope
Environment::Environment(Environment* parentEnvironment)
    : parent(parentEnvironment) {
}

//defines variable in current scope
void Environment::define(const string& name, int value) {
    if (variables.contains(name)) {
        throw runtime_error("Variable already defined: " + name);
    }
    variables[name] = value;
}

//finds variable in current or parent scope
int Environment::get(const string& name) const {
    if (variables.contains(name)) {
        return variables.at(name);
    }
    if (parent != nullptr) {
        return parent->get(name);
    }

    throw runtime_error("Undefined variable: " + name);
}

//updates variable in nearest existing scope
void Environment::assign(const string& name, int value) {
    if (variables.contains(name)) {
        variables[name] = value;
        return;
    }
    if (parent != nullptr) {
        parent->assign(name, value);
        return;
    }

    throw runtime_error("Undefined variable: " + name);
}

//returns variables from current scope
const unordered_map<string, int>& Environment::getVariables() const {
    return variables;
}

//register function in current scope
void Environment::defineFunction(
    const string& name,
    const FunctionDeclaration* declaration
) {
    if (functions.contains(name)) {
        throw runtime_error("Function already defined: " + name);
    }
    functions[name] = {declaration, this};
}

//finds function in current or parent scope
FunctionBinding Environment::getFunction(const string& name) const {
    if (functions.contains(name)) {
        return functions.at(name);
    }
    if (parent != nullptr) {
        return parent->getFunction(name);
    }
    throw runtime_error("Undefined function: " + name);
}