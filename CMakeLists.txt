cmake_minimum_required(VERSION 3.21) # HIP language support requires 3.21
cmake_policy(VERSION 3.21.3...3.27)

project(llvm-note
  VERSION 0.1
  DESCRIPTION "This project is used to record what is leaned from LLVM"
  LANGUAGES C CXX HIP CUDA)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable sanitizers for debug builds by default. If `NOTES_ENABLE_SANITIZER` is
# set, sanitizers are enbaled for all types of builds.
set(NOTES_SANITIZER_FLAGS
  -fsanitize=address -fsanitize=undefined
  CACHE STRING "Compiler Sanitizer Options")
set(NOTES_ENABLE_SANITIZER OFF CACHE BOOL "Enable Sanitizers")

add_subdirectory(deps/googletest)
add_subdirectory(graphs)
#add_subdirectory(llvm_code_example)
add_subdirectory(debugger/ptrace)
add_subdirectory(deps/hipBench)
add_subdirectory(rocm)
