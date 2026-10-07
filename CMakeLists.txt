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
)

target_include_directories(JazzLan PRIVATE include)