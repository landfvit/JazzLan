#pragma once

#include <string>
#include <unordered_map>

class Environment {
private:
    std::unordered_map<std::string, int> variables; //local variables
    Environment* parent; //parent scope

public:
    Environment(Environment* parentEnviroment = nullptr);

    void define(const std::string& name, int value); //create 'x' in current scope
    int get(const std::string& name) const; //find 'x' in current or parent scope
    void assign(const std::string& name, int value); //changes 'x' value in nearest scope

    const std::unordered_map<std::string, int>& getVariables() const;
};