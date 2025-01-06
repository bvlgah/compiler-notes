macro(add_graph_test TESTCASE)
  add_executable(${TESTCASE} ${ARGN})
  target_link_libraries(${TESTCASE}
    PRIVATE graph PRIVATE GTest::gtest_main)
endmacro()

add_graph_test(GraphTest GraphTest.cpp)
add_graph_test(DominatorTest DominatorTest.cpp)
add_graph_test(SCCTest SCCTest.cpp)
add_graph_test(DFSTest DFSTest.cpp)
