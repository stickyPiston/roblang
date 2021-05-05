cmake_minimum_required(VERSION 3.13.4)
project(roblang)

set(CMAKE_BUILD_TYPE Debug)

FILE(GLOB SRCS "${PROJECT_SOURCE_DIR}/src/**/*.c" "${PROJECT_SOURCE_DIR}/src/*.c")
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# Now build our tools
add_executable(${PROJECT_NAME} ${SRCS})
target_include_directories(${PROJECT_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/include")

# Find the libraries that correspond to the LLVM components
# that we wish to use
llvm_map_components_to_libnames(llvm_libs core executionengine mcjit interpreter analysis native bitwriter)

# Link against LLVM libraries
target_link_libraries(roblang ${llvm_libs})