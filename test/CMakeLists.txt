FILE(GLOB LEXER_TESTS "${PROJECT_SOURCE_DIR}/test/lexer.c")
add_executable(lexer_tests ${LEXER_TESTS})
target_link_libraries(lexer_tests roblib)

FILE(GLOB PARSER_TESTS "${PROJECT_SOURCE_DIR}/test/parser.c")
add_executable(parser_tests ${PARSER_TESTS})
target_link_libraries(parser_tests roblib)

include_directories("${PROJECT_SOURCE_DIR}/include")
