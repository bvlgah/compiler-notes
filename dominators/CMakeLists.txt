add_library(dominator SHARED
  src/Graph.cpp src/Dominator.cpp)
target_include_directories(dominator PUBLIC include)

add_subdirectory(test)
