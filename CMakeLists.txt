cmake_minimum_required(VERSION 3.21) # HIP language support requires 3.21
cmake_policy(VERSION 3.21.3...3.27)

set(NOTES_BUILD_ROCM OFF CACHE BOOL "Build ROCm examples")
set(NOTES_BUILD_CUDA OFF CACHE BOOL "Build CUDA examples")

set(notes_languages C CXX)
if(NOTES_BUILD_ROCM)
  list(APPEND notes_languages HIP)
endif(NOTES_BUILD_ROCM)
if(NOTES_BUILD_CUDA)
  list(APPEND notes_languages CUDA)
endif(NOTES_BUILD_CUDA)

project(llvm-note
  VERSION 0.1
  DESCRIPTION "This project is used to record what is leaned from LLVM"
  LANGUAGES ${notes_languages})

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
  "-fsanitize=address -fsanitize=undefined"
  CACHE STRING "Compiler Sanitizer Options")
set(NOTES_ENABLE_SANITIZER OFF CACHE BOOL "Enable Sanitizers")

add_subdirectory(deps/googletest)
add_subdirectory(graphs)
add_subdirectory(llvm_code_example)
add_subdirectory(debugger/ptrace)

if(NOTES_BUILD_ROCM)
  add_subdirectory(deps/hipBench)
  add_subdirectory(rocm)
endif(NOTES_BUILD_ROCM)

if(NOTES_BUILD_CUDA)
  add_subdirectory(cuda)
endif(NOTES_BUILD_CUDA)
