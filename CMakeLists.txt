cmake_minimum_required(VERSION 3.13)

project(llvm-note
  VERSION 0.1
  DESCRIPTION "This project is used to record what is leaned from LLVM"
  LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(deps/googletest)
add_subdirectory(dominators)
add_subdirectory(llvm_code_example)
