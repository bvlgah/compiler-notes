cmake_minimum_required(VERSION 3.20.0)

find_package(LLVM 19 REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

message(STATUS "CMake module path: ${CMAKE_MODULE_PATH}")
include(AddLLVM)

add_llvm_pass_plugin(LLVMHello
  src/Driver.cpp
  src/LiveVariableAnalysis.cpp
  src/Loop.cpp
  src/SCCP.cpp)
