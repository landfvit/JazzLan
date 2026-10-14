#generates isolated source and expected output files
function(add_jazz_test name source output error)
    set(testPath "${CMAKE_CURRENT_BINARY_DIR}/${name}")
    file(WRITE "${testPath}.jazz" "${source}")
    file(WRITE "${testPath}.expected" "${output}")

    if("${error}" STREQUAL "")
        set(exitCode 0)
        file(WRITE "${testPath}.error" "")
    else()
        set(exitCode 1)
        file(WRITE "${testPath}.error" "JazzLanError: ${error}\n")
    endif()

    add_test(
        NAME ${name}
        COMMAND ${CMAKE_COMMAND}
            "-DPROGRAM=$<TARGET_FILE:JazzLan>"
            "-DSOURCE=${testPath}.jazz"
            "-DEXPECTED_OUTPUT=${testPath}.expected"
            "-DEXPECTED_ERROR=${testPath}.error"
            "-DEXPECTED_EXIT_CODE=${exitCode}"
            "-DOPTIONS=${ARGN}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/run_test.cmake"
    )
endfunction()

#keeps existing examples covered
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/types.jazz" typesSource)
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/types.expected" typesOutput)
add_jazz_test(types "${typesSource}" "${typesOutput}" "")

file(READ "${PROJECT_SOURCE_DIR}/examples/factorial.jazz" factorialSource)
add_jazz_test(factorial "${factorialSource}" "120\n" "")
add_jazz_test(arithmetic "print(3 + 2 * 4);" "11\n" "")

file(READ "${PROJECT_SOURCE_DIR}/examples/function_types.jazz" functionTypesSource)
add_jazz_test(function_types_example
    "${functionTypesSource}" "none\n42\nJazz\nnone\n5.5\nHello Jazz\n" ""
)

#none is a value and has its own fixed type
add_jazz_test(none_values [[
var inferred = none;
var explicit: none = none;
var delayed: none;
delayed = none;
const empty = none;
inferred = delayed;
print(inferred);
print(explicit);
print(empty);
print(none == none);
print(none != none);
print(none != 0);
print(0 == none);
print(none != false);
print(none != "none");
]] "none\nnone\nnone\ntrue\nfalse\ntrue\nfalse\ntrue\ntrue\n" "")

#parameter and return annotations are independently optional
add_jazz_test(function_annotations [[
function integer(value: int) {
    return value;
}
function decimal(value: float): float {
    fetch value;
}
function text(value): string {
    return value;
}
function boolean(value: bool): bool {
    return value;
}
function empty(value: none): none {
    fetch value;
}
function mixed(a: int, b): float {
    return a + b;
}
print(integer(5));
print(decimal(2.5));
print(text("Jazz"));
print(boolean(true));
print(empty(none));
print(mixed(2, 0.5));
]] "5\n2.5\nJazz\ntrue\nnone\n2.5\n" "")

#unannotated functions infer parameters and results for each call
add_jazz_test(function_inference [[
function identity(value) {
    return value;
}
print(identity(7));
print(identity(3.5));
print(identity("Jazz"));
print(identity(false));
print(identity(none));
function choose(value) {
    if (value) {
        return 8;
    }
    return;
}
choose(false);
print(choose(true));
]] "7\n3.5\nJazz\nfalse\nnone\n8\n" "")

#returns leave only the current call, including nested blocks
add_jazz_test(function_return_flow [[
function nested(): void {
    if (true) {
        fetch;
    }
    print("unreachable");
}
function outer(value: int): int {
    nested();
    while (value > 0) {
        if (value == 2) {
            return value;
        }
        value = value - 1;
    }
    return 0;
}
function factorial(value: int): int {
    if (value <= 1) {
        return 1;
    }
    return value * factorial(value - 1);
}
function empty(): none {
    while (true) {
        fetch none;
    }
}
print(outer(4));
print(outer(0));
print(factorial(5));
print(empty());
]] "2\n0\n120\nnone\n" "")

#void allows falling through and both bare return keywords
add_jazz_test(void_functions [[
function finish(): void {
    print("finish");
}
function early(): void {
    print("early");
    return;
    print("unreachable");
}
function fetchEarly(): void {
    print("fetch");
    fetch;
    print("unreachable");
}
function inferred() {
    fetch;
    print("unreachable");
}
finish();
early();
fetchEarly();
inferred();
print("done");
]] "finish\nearly\nfetch\ndone\n" "")

add_jazz_test(none_uninitialized
    "var empty: none; print(empty);" "" "Uninitialized variable: empty"
)
add_jazz_test(none_assignment
    "var empty = none; empty = 1;" "" "Type mismatch: empty"
)
add_jazz_test(none_initializer
    "var empty: none = 1;" "" "Type mismatch: empty"
)
add_jazz_test(none_const
    "const empty = none; empty = none;" "" "Cannot assign to const: empty"
)
add_jazz_test(none_arithmetic
    "print(none + 1);" "" "Operator '+' requires two numbers or two strings"
)
add_jazz_test(none_condition
    "if (none) { print(1); }" "" "Condition must be bool or number"
)
add_jazz_test(none_comparison
    "print(none < 1);" "" "Comparison requires numeric operands"
)

#parameter values must have exactly the annotated type
add_jazz_test(parameter_float_rejects_int
    "function value(input: float) { return input; } value(1);"
    "" "Argument type mismatch: value (parameter input, expected float, got int)"
)
add_jazz_test(parameter_int_rejects_float
    "function value(input: int) { return input; } value(1.0);"
    "" "Argument type mismatch: value (parameter input, expected int, got float)"
)
add_jazz_test(parameter_none_rejects_int
    "function value(input: none) { return input; } value(1);"
    "" "Argument type mismatch: value (parameter input, expected none, got int)"
)
add_jazz_test(parameter_int_rejects_none
    "function value(input: int) { return input; } value(none);"
    "" "Argument type mismatch: value (parameter input, expected int, got none)"
)
add_jazz_test(parameter_inferred_fixed_type
    "function value(input) { input = none; } value(1);"
    "" "Type mismatch: input"
)
add_jazz_test(parameter_duplicate
    "function value(input: int, input: none) {}"
    "" "Duplicate parameter: input"
)
add_jazz_test(parameter_argument_count
    "function value(input: none): void {} value();"
    "" "Invalid argument count: value"
)

#return annotations are checked even for standalone calls
foreach(type int float string bool none)
    add_jazz_test(return_${type}_missing
        "function value(): ${type} {} value();"
        "" "Function must return ${type}: value"
    )
    add_jazz_test(return_${type}_bare
        "function value(): ${type} { return; } value();"
        "" "Function must return ${type}: value"
    )
endforeach()

add_jazz_test(return_int_rejects_float
    "function value(): int { return 1.0; } value();"
    "" "Return type mismatch: value (expected int, got float)"
)
add_jazz_test(return_float_rejects_int
    "function value(): float { return 1; } value();"
    "" "Return type mismatch: value (expected float, got int)"
)
add_jazz_test(return_none_rejects_int
    "function value(): none { return 1; } value();"
    "" "Return type mismatch: value (expected none, got int)"
)
add_jazz_test(return_int_rejects_none
    "function value(): int { return none; } value();"
    "" "Return type mismatch: value (expected int, got none)"
)
add_jazz_test(return_branch_missing
    "function value(flag: bool): int { if (flag) { return 1; } } print(value(true)); value(false);"
    "1\n" "Function must return int: value"
)
add_jazz_test(return_nested_mismatch
    "function inner(): int { return none; } function outer(): none { return inner(); } outer();"
    "" "Return type mismatch: inner (expected int, got none)"
)

#void is only a return annotation and cannot supply an expression value
add_jazz_test(void_variable
    "var value: void;" "" "Void is only allowed as a function return type"
)
add_jazz_test(void_parameter
    "function value(input: void) {}" "" "Void is only allowed as a function return type"
)
add_jazz_test(void_returns_none
    "function value(): void { fetch none; } value();"
    "" "Void function cannot return a value: value"
)
add_jazz_test(void_returns_int
    "function value(): void { return 1; } value();"
    "" "Void function cannot return a value: value"
)
add_jazz_test(void_print
    "function value(): void {} print(value());"
    "" "Function returned no value: value"
)
add_jazz_test(void_initializer
    "function value(): void { return; } var empty = value();"
    "" "Function returned no value: value"
)
add_jazz_test(void_argument
    "function value(): void {} function identity(input: none): none { return input; } identity(value());"
    "" "Function returned no value: value"
)

foreach(keyword return fetch)
    add_jazz_test(${keyword}_outside_function
        "${keyword};" "" "'return' or 'fetch' outside function"
    )
    add_jazz_test(${keyword}_unannotated_expression
        "function value() { ${keyword}; } print(value());"
        "" "Function returned no value: value"
    )
endforeach()

#debug output distinguishes none, missing values and return annotations
set(astOutput "\n")
string(APPEND astOutput [[Program
├── VariableDeclaration
│   ├── name: empty
│   └── value:
│       └── NoneExpression(none)
├── FunctionDeclaration
│   ├── name: value
│   ├── parameters: [input: int, rest]
│   ├── returnType: none
│   └── body:
│       └── ReturnStatement
│           └── value:
│               └── NoneExpression(none)
└── FunctionDeclaration
    ├── name: finish
    ├── parameters: []
    ├── returnType: void
    └── body:
        └── ReturnStatement
            └── no value
]])
add_jazz_test(function_types_ast [[
var empty = none;
function value(input: int, rest): none {
    fetch none;
}
function finish(): void {
    return;
}
]] "${astOutput}" "" --ast)

include("${CMAKE_CURRENT_SOURCE_DIR}/containers.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/transforms.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/loops.cmake")
