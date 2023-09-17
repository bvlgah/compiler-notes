#include <cstdlib>
#include <gtest/gtest.h>
#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "Graph.h"

class GraphTest : public ::testing::Test {
protected:
  void SetUp() override {
    Test::SetUp();

    for (std::size_t i = 0; i < m_numNodes; i++) {
      std::string name = "node " + std::to_string(i);
      m_nodes.push_back(m_graph.createNode(name.c_str()));
    }

    m_graph.connect(m_nodes[0], m_nodes[1]);
    m_graph.connect(m_nodes[1], m_nodes[2]);
    m_graph.connect(m_nodes[2], m_nodes[3]);
    m_graph.connect(m_nodes[1], m_nodes[4]);
    m_graph.connect(m_nodes[4], m_nodes[5]);
    m_graph.connect(m_nodes[3], m_nodes[6]);
    m_graph.connect(m_nodes[3], m_nodes[5]);
    m_graph.connect(m_nodes[2], m_nodes[0]);
    m_graph.connect(m_nodes[3], m_nodes[1]);
    m_graph.connect(m_nodes[0], m_nodes[6]);
    m_graph.connect(m_nodes[6], m_nodes[4]);
    m_graph.connect(m_nodes[5], m_nodes[6]);
    m_graph.connect(m_nodes[3], m_nodes[1]);
    m_graph.connect(m_nodes[5], m_nodes[2]);
  }

  std::size_t m_numNodes = 7;
  std::vector<GraphNodeSP> m_nodes {};
  Graph m_graph {};
};

TEST_F(GraphTest, TestIterator) {
  std::set<GraphNodeSP> nodesIterated(m_graph.begin(), m_graph.end());
  std::set<GraphNodeSP> nodesExpected(m_nodes.begin(), m_nodes.end());
  ASSERT_EQ(nodesIterated, nodesExpected);
}

TEST_F(GraphTest, TestPredecessor) {
  std::set<GraphNodeSP> pred0(m_nodes[0]->predBegin(), m_nodes[0]->predEnd());
  std::set<GraphNodeSP> pred0Expected { m_nodes[2] };
  ASSERT_EQ(pred0, pred0Expected);

  std::set<GraphNodeSP> pred1(m_nodes[1]->predBegin(), m_nodes[1]->predEnd());
  std::set<GraphNodeSP> pred1Expected { m_nodes[0], m_nodes[3] };
  ASSERT_EQ(pred1, pred1Expected);

  std::set<GraphNodeSP> pred2(m_nodes[2]->predBegin(), m_nodes[2]->predEnd());
  std::set<GraphNodeSP> pred2Expected { m_nodes[1], m_nodes[5] };
  ASSERT_EQ(pred2, pred2Expected);

  std::set<GraphNodeSP> pred3(m_nodes[3]->predBegin(), m_nodes[3]->predEnd());
  std::set<GraphNodeSP> pred3Expected { m_nodes[2] };
  ASSERT_EQ(pred3, pred3Expected);

  std::set<GraphNodeSP> pred4(m_nodes[4]->predBegin(), m_nodes[4]->predEnd());
  std::set<GraphNodeSP> pred4Expected { m_nodes[1], m_nodes[6] };
  ASSERT_EQ(pred4, pred4Expected);

  std::set<GraphNodeSP> pred5(m_nodes[5]->predBegin(), m_nodes[5]->predEnd());
  std::set<GraphNodeSP> pred5Expected { m_nodes[4], m_nodes[3] };
  ASSERT_EQ(pred5, pred5Expected);

  std::set<GraphNodeSP> pred6(m_nodes[6]->predBegin(), m_nodes[6]->predEnd());
  std::set<GraphNodeSP> pred6Expected { m_nodes[3], m_nodes[0], m_nodes[5] };
  ASSERT_EQ(pred6, pred6Expected);
}

TEST_F(GraphTest, TestSuccessor) {
  std::set<GraphNodeSP> succ0(m_nodes[0]->succBegin(), m_nodes[0]->succEnd());
  std::set<GraphNodeSP> succ0Expected { m_nodes[1], m_nodes[6] };
  ASSERT_EQ(succ0, succ0Expected);

  std::set<GraphNodeSP> succ1(m_nodes[1]->succBegin(), m_nodes[1]->succEnd());
  std::set<GraphNodeSP> succ1Expected { m_nodes[2], m_nodes[4] };
  ASSERT_EQ(succ1, succ1Expected);

  std::set<GraphNodeSP> succ2(m_nodes[2]->succBegin(), m_nodes[2]->succEnd());
  std::set<GraphNodeSP> succ2Expected { m_nodes[3], m_nodes[0] };
  ASSERT_EQ(succ2, succ2Expected);

  std::set<GraphNodeSP> succ3(m_nodes[3]->succBegin(), m_nodes[3]->succEnd());
  std::set<GraphNodeSP> succ3Expected { m_nodes[6], m_nodes[5], m_nodes[1] };
  ASSERT_EQ(succ3, succ3Expected);

  std::set<GraphNodeSP> succ4(m_nodes[4]->succBegin(), m_nodes[4]->succEnd());
  std::set<GraphNodeSP> succ4Expected { m_nodes[5] };
  ASSERT_EQ(succ4, succ4Expected);

  std::set<GraphNodeSP> succ5(m_nodes[5]->succBegin(), m_nodes[5]->succEnd());
  std::set<GraphNodeSP> succ5Expected { m_nodes[6], m_nodes[2] };
  ASSERT_EQ(succ5, succ5Expected);

  std::set<GraphNodeSP> succ6(m_nodes[6]->succBegin(), m_nodes[6]->succEnd());
  std::set<GraphNodeSP> succ6Expected { m_nodes[4] };
  ASSERT_EQ(succ6, succ6Expected);
}
