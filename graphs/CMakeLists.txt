if(NOTES_ENABLE_SANITIZER)
  add_compile_options(${NOTES_SANITIZER_FLAGS})
endif()

add_library(graph SHARED
  src/Graph.cpp src/Dominator.cpp src/SCC.cpp)
target_include_directories(graph PUBLIC include)

add_subdirectory(test)
