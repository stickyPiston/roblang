cmake_minimum_required(VERSION 3.13.4)

set(CMAKE_BUILD_TYPE Debug)

FILE(GLOB SRCS "${PROJECT_SOURCE_DIR}/src/**/*.c" "${PROJECT_SOURCE_DIR}/src/*.c")
list(REMOVE_ITEM SRCS "${PROJECT_SOURCE_DIR}/src/main.c")

include_directories("${PROJECT_SOURCE_DIR}/include")

# Now build our tools
add_executable(roblang "${PROJECT_SOURCE_DIR}/src/main.c")
add_library(roblib ${SRCS})
target_link_libraries (roblang roblib)
target_include_directories(roblang PRIVATE "${PROJECT_SOURCE_DIR}/include")
