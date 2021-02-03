FILE(GLOB TESTS "${PROJECT_SOURCE_DIR}/test/**/*.c" "${PROJECT_SOURCE_DIR}/test/*.c")
add_executable(robtest ${TESTS})
target_link_libraries(robtest roblib)
include_directories("${PROJECT_SOURCE_DIR}/include")
