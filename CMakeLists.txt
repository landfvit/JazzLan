cmake_minimum_required(VERSION 3.20)

project(JazzLan)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(JazzLan
    src/main.cpp
    src/Lexer.cpp
    src/Parser.cpp
    src/AST.cpp
    src/Interpreter.cpp
    src/Environment.cpp
    src/Value.cpp
)

target_include_directories(JazzLan PRIVATE include)

enable_testing()

add_test(
    NAME arithmetic
    COMMAND $<TARGET_FILE:JazzLan>
            ${CMAKE_SOURCE_DIR}/tests/arithmetic.jazz
)

set_tests_properties(arithmetic PROPERTIES
    PASS_REGULAR_EXPRESSION "11"
)

add_test(
    NAME factorial
    COMMAND $<TARGET_FILE:JazzLan>
            ${CMAKE_SOURCE_DIR}/tests/factorial.jazz
)

set_tests_properties(factorial PROPERTIES
    PASS_REGULAR_EXPRESSION "120"
)