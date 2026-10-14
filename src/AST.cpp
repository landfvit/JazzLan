#include "AST.hpp"
#include <iostream>

using namespace std;

//recursively prints one expression node as a tree
void printExpressionTree(
    const Expression* expression,
    const string& prefix,
    bool isLast,
    const std::unordered_map<std::string, VariableInfo>& variables
) {
    cout << prefix;
    cout << (isLast ? "└── " : "├── ");

    //uninitialized expression
    if (expression == nullptr) {
        cout << "Uninitialized" << endl;
        return;
    }

    //number node
    if (const auto* number =
        dynamic_cast<const NumberExpression*>(expression)) {

        cout << "NumberExpression(" << number->value << ")" << endl;
        return;
    }

    //identifier node
    if (const auto* identifier =
        dynamic_cast<const IdentifierExpression*>(expression)) {
        cout << "IdentifierExpression(" << identifier->name;

        cout << ")" << endl;
        return;
    }

    //binary op. node
    if (const auto* binary =
        dynamic_cast<const BinaryExpression*>(expression)) {

        cout << "BinaryExpression(" << binary->op << ")" << endl;

        //keeps the tree lines aligned for child nodes
        string childPrefix =
            prefix + (isLast ? "    " : "│   ");

        printExpressionTree(
            binary->left.get(),
            childPrefix,
            false,
            variables
        );

        printExpressionTree(
            binary->right.get(),
            childPrefix,
            true,
            variables
        );

        return;
    }
    //function call
    if (const auto* call =
        dynamic_cast<const CallExpression*>(expression)) {

        cout << "CallExpression(" << call->name << ")" << endl;
        if (!call->arguments.empty()) {
            string childPrefix =
                prefix + (isLast ? "    " : "│   ");

            cout << childPrefix << "└── arguments:" << endl;

            for (size_t i = 0; i < call->arguments.size(); i++) {
                printExpressionTree(
                    call->arguments[i].get(),
                    childPrefix + "    ",
                    i + 1 == call->arguments.size(),
                    variables
                );
            }
        }
    return;
    }
    //float node
    if (const auto* number =
        dynamic_cast<const FloatExpression*>(expression)) {

        cout << "FloatExpression(" << number->value << ")" << endl;
        return;
    }
    //string node
    if (const auto* text =
        dynamic_cast<const StringExpression*>(expression)) {

        cout << "StringExpression(\"" << text->value << "\")" << endl;
        return;
    }
    //boolean node
    if (const auto* boolean =
        dynamic_cast<const BoolExpression*>(expression)) {

        cout << "BoolExpression(";
        cout << (boolean->value ? "true" : "false") << ")" << endl;
        return;
    }

    //none node
    if (dynamic_cast<const NoneExpression*>(expression)) {
        cout << "NoneExpression(none)" << endl;
        return;
    }
    //transform placeholder
    if (dynamic_cast<const PlaceholderExpression*>(expression)) {
        cout << "PlaceholderExpression(#)" << endl;
        return;
    }

    string childPrefix = prefix + (isLast ? "    " : "│   ");

    //container literal
    if (const auto* container =
        dynamic_cast<const ContainerExpression*>(expression)) {

        cout << "ContainerExpression(" << valueTypeToString(container->type) << ")" << endl;

        for (size_t i = 0; i < container->elements.size(); i++) {
            printExpressionTree(
                container->elements[i].get(),
                childPrefix,
                i + 1 == container->elements.size(),
                variables
            );
        }
        return;
    }
    //dictionary literal
    if (const auto* dictionary =
        dynamic_cast<const DictionaryExpression*>(expression)) {

        cout << "DictionaryExpression" << endl;

        for (size_t i = 0; i < dictionary->entries.size(); i++) {
            bool lastEntry = i + 1 == dictionary->entries.size();
            cout << childPrefix << (lastEntry ? "└── entry:" : "├── entry:") << endl;
            string entryPrefix = childPrefix + (lastEntry ? "    " : "│   ");
            printExpressionTree(dictionary->entries[i].first.get(), entryPrefix, false, variables);
            printExpressionTree(dictionary->entries[i].second.get(), entryPrefix, true, variables);
        }
        return;
    }
    //unary operator
    if (const auto* unary =
        dynamic_cast<const UnaryExpression*>(expression)) {

        cout << "UnaryExpression(" << unary->op << ")" << endl;
        printExpressionTree(unary->operand.get(), childPrefix, true, variables);
        return;
    }
    //chained comparison
    if (const auto* comparison =
        dynamic_cast<const ComparisonExpression*>(expression)) {

        cout << "ComparisonExpression(";

        for (size_t i = 0; i < comparison->operators.size(); i++) {
            if (i != 0) {
                cout << ", ";
            }
            cout << comparison->operators[i];
        }
        cout << ")" << endl;

        for (size_t i = 0; i < comparison->operands.size(); i++) {
            printExpressionTree(
                comparison->operands[i].get(),
                childPrefix,
                i + 1 == comparison->operands.size(),
                variables
            );
        }
        return;
    }
    //element access
    if (const auto* index =
        dynamic_cast<const IndexExpression*>(expression)) {

        cout << "IndexExpression" << endl;
        printExpressionTree(index->object.get(), childPrefix, false, variables);
        printExpressionTree(index->index.get(), childPrefix, true, variables);
        return;
    }
    //sequence slice
    if (const auto* slice =
        dynamic_cast<const SliceExpression*>(expression)) {

        cout << "SliceExpression" << endl;
        printExpressionTree(slice->object.get(), childPrefix, false, variables);

        const Expression* bounds[] = {slice->start.get(), slice->stop.get(), slice->step.get()};
        const char* names[] = {"start", "stop", "step"};

        for (size_t i = 0; i < 3; i++) {
            bool lastBound = i == 2;
            cout << childPrefix << (lastBound ? "└── " : "├── ") << names[i] << ":";

            if (bounds[i] == nullptr) {
                cout << " default" << endl;
            } else {
                cout << endl;
                printExpressionTree(
                    bounds[i], childPrefix + (lastBound ? "    " : "│   "), true, variables
                );
            }
        }
        return;
    }
    //method call
    if (const auto* call =
        dynamic_cast<const MethodCallExpression*>(expression)) {

        cout << "MethodCallExpression(" << call->name << ")" << endl;
        printExpressionTree(call->object.get(), childPrefix, call->arguments.empty(), variables);

        for (size_t i = 0; i < call->arguments.size(); i++) {
            printExpressionTree(
                call->arguments[i].get(), childPrefix, i + 1 == call->arguments.size(), variables
            );
        }
        return;
    }

    cout << "UnknownExpression" << endl;
}

//prints an expression from its root node
void printExpression(
    const Expression* expression,
    const std::unordered_map<std::string, VariableInfo>& variables
) {
    printExpressionTree(
        expression,
        "",
        true,
        variables
    );
}

//prints one variable declaration and its expression tree
void printVariableDeclaration(
    const VariableDeclaration& variable,
    const std::unordered_map<std::string, VariableInfo>& variables
) {
    cout << "VariableDeclaration" << endl;
    cout << "├── name: " << variable.name << endl;
    cout << "└── value:" << endl;

    printExpressionTree(
        variable.value.get(),
        "    ",
        true,
        variables
    );
}

//recursively prints statements and nested branches
void printStatementTree(
    const Statement* statement,
    const string& prefix,
    bool isLast,
    const std::unordered_map<std::string, VariableInfo>& variables
) {
    cout << prefix << (isLast ? "└── " : "├── ");

    string childPrefix = prefix + (isLast ? "    " : "│   ");
    //variable declaration
    if (const auto* variable =
        dynamic_cast<const VariableDeclaration*>(statement)) {

        cout << "VariableDeclaration" << endl;
        cout << childPrefix << "├── name: " << variable->name << endl;
        cout << childPrefix << "└── value:" << endl;

        printExpressionTree(
            variable->value.get(),
            childPrefix + "    ",
            true,
            variables
        );
        return;
    }
    //print statement
    if (const auto* print =
        dynamic_cast<const PrintStatement*>(statement)) {

        cout << "PrintStatement" << endl;
        cout << childPrefix << "└── value:" << endl;

        printExpressionTree(
            print->value.get(),
            childPrefix + "    ",
            true,
            variables
        );

        return;
    }
    //if/else statement
    if (const auto* ifStatement =
        dynamic_cast<const IfStatement*>(statement)) {

        cout << "IfStatement" << endl;

        cout << childPrefix << "├── condition:" << endl;

        printExpressionTree(
            ifStatement->condition.get(),
            childPrefix + "│   ",
            true,
            variables
        );

        bool hasElse = !ifStatement->elseBranch.empty();

        cout << childPrefix
             << (hasElse ? "├── thenBranch:" : "└── thenBranch:")
             << endl;

        string thenPrefix = childPrefix + (hasElse ? "│   " : "    ");

        for (size_t i = 0; i < ifStatement->thenBranch.size(); i++) {
            printStatementTree(
                ifStatement->thenBranch[i].get(),
                thenPrefix,
                i + 1 == ifStatement->thenBranch.size(),
                variables
            );
        }
        if (hasElse) {
            cout << childPrefix << "└── elseBranch:" << endl;

            for (size_t i = 0; i < ifStatement->elseBranch.size(); i++) {
                printStatementTree(
                    ifStatement->elseBranch[i].get(),
                    childPrefix + "    ",
                    i + 1 == ifStatement->elseBranch.size(),
                    variables
                );
            }
        }
        return;
    }
    //variable assignment
    if (const auto* assignment =
        dynamic_cast<const AssignmentStatement*>(statement)) {

        cout << "AssignmentStatement" << endl;
        cout << childPrefix << "├── name: " << assignment->name << endl;
        cout << childPrefix << "└── value:" << endl;

        printExpressionTree(
            assignment->value.get(),
            childPrefix + "    ",
            true,
            variables
        );

        return;
    }
    //container element assignment
    if (const auto* assignment =
        dynamic_cast<const IndexAssignmentStatement*>(statement)) {

        cout << "IndexAssignmentStatement" << endl;
        printExpressionTree(assignment->object.get(), childPrefix, false, variables);
        printExpressionTree(assignment->index.get(), childPrefix, false, variables);
        printExpressionTree(assignment->value.get(), childPrefix, true, variables);
        return;
    }
    //container transform
    if (const auto* transform =
        dynamic_cast<const TransformStatement*>(statement)) {

        cout << "TransformStatement(" << valueTypeToString(transform->type) << ")" << endl;
        cout << childPrefix << "├── source:" << endl;
        printExpressionTree(transform->source.get(), childPrefix + "│   ", true, variables);
        cout << childPrefix << "├── expression:" << endl;
        printExpressionTree(transform->expression.get(), childPrefix + "│   ", true, variables);
        cout << childPrefix << "└── target: ";

        if (transform->target.has_value()) {
            cout << transform->target.value() << endl;
        } else {
            cout << "in place" << endl;
        }
        return;
    }
    //while loop
    if (const auto* loop =
        dynamic_cast<const WhileStatement*>(statement)) {

        cout << "WhileStatement" << endl;

        //condition
        cout << childPrefix << "├── condition:" << endl;

        printExpressionTree(
            loop->condition.get(),
            childPrefix + "│   ",
            true,
            variables
        );

        //loop body
        cout << childPrefix << "└── body:" << endl;

        for (size_t i = 0; i < loop->body.size(); i++) {
            printStatementTree(
                loop->body[i].get(),
                childPrefix + "    ",
                i + 1 == loop->body.size(),
                variables
            );
        }

        return;
    }
    //for loop
    if (const auto* loop =
        dynamic_cast<const ForStatement*>(statement)) {

        cout << "ForStatement" << endl;
        cout << childPrefix << "├── name: " << loop->name << endl;

        if (loop->declaredType.has_value()) {
            cout << childPrefix << "├── type: ";
            cout << valueTypeToString(loop->declaredType.value()) << endl;
        }

        cout << childPrefix << "├── iterable:" << endl;
        printExpressionTree(loop->iterable.get(), childPrefix + "│   ", true, variables);
        cout << childPrefix << "└── body:" << endl;

        for (size_t i = 0; i < loop->body.size(); i++) {
            printStatementTree(
                loop->body[i].get(),
                childPrefix + "    ",
                i + 1 == loop->body.size(),
                variables
            );
        }
        return;
    }
    //loop controls
    if (dynamic_cast<const BreakStatement*>(statement)) {
        cout << "BreakStatement" << endl;
        return;
    }
    if (dynamic_cast<const ContinueStatement*>(statement)) {
        cout << "ContinueStatement" << endl;
        return;
    }
    //return statement
    if (const auto* returnStatement =
        dynamic_cast<const ReturnStatement*>(statement)) {

        cout << "ReturnStatement" << endl;

        if (!returnStatement->value) {
            cout << childPrefix << "└── no value" << endl;
            return;
        }

        cout << childPrefix << "└── value:" << endl;

        printExpressionTree(
            returnStatement->value.get(),
            childPrefix + "    ",
            true,
            variables
        );

        return;
    }
    //function declaration
    if (const auto* function =
        dynamic_cast<const FunctionDeclaration*>(statement)) {

        cout << "FunctionDeclaration" << endl;
        cout << childPrefix << "├── name: " << function->name << endl;

        //function parameters
        cout << childPrefix << "├── parameters: [";

        for (size_t i = 0; i < function->parameters.size(); i++) {
            const auto& parameter = function->parameters[i];
            cout << parameter.name;

            if (parameter.declaredType.has_value()) {
                cout << ": " << valueTypeToString(parameter.declaredType.value());
            }

            if (i + 1 < function->parameters.size()) {
                cout << ", ";
            }
        }

        cout << "]" << endl;

        //optional return type
        if (function->returnType.has_value()) {
            cout << childPrefix << "├── returnType: ";
            cout << valueTypeToString(function->returnType.value()) << endl;
        }

        cout << childPrefix << "└── body:" << endl;

        //function body
        for (size_t i = 0; i < function->body.size(); i++) {
            printStatementTree(
                function->body[i].get(),
                childPrefix + "    ",
                i + 1 == function->body.size(),
                variables
            );
        }

        return;
    }
    //expression statement
    if (const auto* expressionStatement =
        dynamic_cast<const ExpressionStatement*>(statement)) {

        cout << "ExpressionStatement" << endl;
        cout << childPrefix << "└── value:" << endl;

        printExpressionTree(
            expressionStatement->value.get(),
            childPrefix + "    ",
            true,
            variables
        );

        return;
    }
    cout << "UnknownStatement" << endl;
}

//prints the whole program AST
void printProgram(
    const Program& program,
    const std::unordered_map<std::string, VariableInfo>& variables
) {
    cout << "Program" << endl;

    for (size_t i = 0; i < program.statements.size(); i++) {
        printStatementTree(
            program.statements[i].get(),
            "",
            i + 1 == program.statements.size(),
            variables
        );
    }
}
