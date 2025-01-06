if(NOTES_ENABLE_SANITIZER)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${NOTES_SANITIZER_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${NOTES_SANITIZER_FLAGS}")
endif()

add_library(graph SHARED
  src/Graph.cpp src/Dominator.cpp src/SCC.cpp)
target_include_directories(graph PUBLIC include)

add_subdirectory(test)
