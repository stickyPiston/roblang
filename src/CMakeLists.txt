cmake_minimum_required(VERSION 3.13.4)

set(CMAKE_BUILD_TYPE Debug)

FILE(GLOB SRCS "${PROJECT_SOURCE_DIR}/src/**/*.c" "${PROJECT_SOURCE_DIR}/src/*.c")
list(REMOVE_ITEM SRCS "${PROJECT_SOURCE_DIR}/src/main.c")
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories("${PROJECT_SOURCE_DIR}/include")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# Now build our tools
add_executable(roblang "${PROJECT_SOURCE_DIR}/src/main.c")
add_library(roblib ${SRCS})
target_link_libraries (roblang roblib)
target_include_directories(roblang PRIVATE "${PROJECT_SOURCE_DIR}/include")

# Find the libraries that correspond to the LLVM components
# that we wish to use
llvm_map_components_to_libnames(llvm_libs core executionengine mcjit interpreter analysis native bitwriter)

# Link against LLVM libraries
target_link_libraries(roblang ${llvm_libs})
