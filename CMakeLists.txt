cmake_minimum_required(VERSION 3.13.4)
project(roblang)

add_subdirectory(src) 
add_subdirectory(test)

enable_testing()
add_test(NAME lexer COMMAND lexer_tests)
add_test(NAME parser COMMAND parser_tests)