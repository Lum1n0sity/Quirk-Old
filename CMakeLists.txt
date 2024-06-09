cmake_minimum_required(VERSION 3.13)
project(QuirkCompilerCpp)

find_package(LLVM REQUIRED CONFIG)
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_executable(QuirkCompilerCpp src/main.cpp src/parser.cpp src/codegen.cpp)

llvm_map_components_to_libnames(llvm_libs core irreader)

target_link_libraries(QuirkCompilerCpp ${llvm_libs})
