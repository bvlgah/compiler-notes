add_executable(GraphTest GraphTest.cpp)
target_link_libraries(GraphTest
  PRIVATE dominator
  PRIVATE GTest::gtest_main)
add_executable(DominatorTest DominatorTest.cpp)
target_link_libraries(DominatorTest
  PRIVATE dominator
  PRIVATE GTest::gtest_main)
