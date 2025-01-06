#include <cstdlib>
#include <iostream>
#include <gtest/gtest.h>
#include <unordered_map>
#include <vector>

#include "Dominator.h"

class DominatorTreeTest : public ::testing::Test {
protected:
  void SetUp() override {
    Test::SetUp();

    for (char n: m_nodeNames) {
      std::string name = "node ";
      name.push_back(n);
      m_nodes.emplace(n, &m_graph.createNode(name.c_str()));
    }

    m_graph.connect(*m_nodes['R'], *m_nodes['C']);
    m_graph.connect(*m_nodes['R'], *m_nodes['B']);
    m_graph.connect(*m_nodes['R'], *m_nodes['A']);
    m_graph.connect(*m_nodes['A'], *m_nodes['D']);
    m_graph.connect(*m_nodes['B'], *m_nodes['E']);
    m_graph.connect(*m_nodes['B'], *m_nodes['A']);
    m_graph.connect(*m_nodes['B'], *m_nodes['D']);
    m_graph.connect(*m_nodes['C'], *m_nodes['F']);
    m_graph.connect(*m_nodes['C'], *m_nodes['G']);
    m_graph.connect(*m_nodes['D'], *m_nodes['L']);
    m_graph.connect(*m_nodes['E'], *m_nodes['H']);
    m_graph.connect(*m_nodes['F'], *m_nodes['I']);
    m_graph.connect(*m_nodes['G'], *m_nodes['I']);
    m_graph.connect(*m_nodes['G'], *m_nodes['J']);
    m_graph.connect(*m_nodes['H'], *m_nodes['K']);
    m_graph.connect(*m_nodes['H'], *m_nodes['E']);
    m_graph.connect(*m_nodes['I'], *m_nodes['K']);
    m_graph.connect(*m_nodes['J'], *m_nodes['I']);
    m_graph.connect(*m_nodes['K'], *m_nodes['I']);
    m_graph.connect(*m_nodes['K'], *m_nodes['R']);
    m_graph.connect(*m_nodes['L'], *m_nodes['H']);

    m_domTree.reconstruct(m_graph, *m_nodes['R']);
  }

  Graph m_graph {};
  std::unordered_map<char, GraphNode *> m_nodes {};
  std::vector<char> m_nodeNames {
    'R', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L'
  };
  DominatorTree m_domTree {};
};

TEST_F(DominatorTreeTest, TestDominates) {
  ASSERT_TRUE(m_domTree.dominates(*m_nodes['R'], *m_nodes['A']));
  ASSERT_TRUE(m_domTree.dominates(*m_nodes['R'], *m_nodes['B']));
  ASSERT_TRUE(m_domTree.dominates(*m_nodes['R'], *m_nodes['C']));
  ASSERT_TRUE(m_domTree.dominates(*m_nodes['R'], *m_nodes['D']));
  ASSERT_TRUE(m_domTree.dominates(*m_nodes['R'], *m_nodes['E']));
  ASSERT_TRUE(m_domTree.dominates(*m_nodes['R'], *m_nodes['F']));
  ASSERT_TRUE(m_domTree.dominates(*m_nodes['R'], *m_nodes['G']));
  ASSERT_TRUE(m_domTree.dominates(*m_nodes['R'], *m_nodes['H']));
  ASSERT_TRUE(m_domTree.dominates(*m_nodes['R'], *m_nodes['I']));
  ASSERT_TRUE(m_domTree.dominates(*m_nodes['R'], *m_nodes['J']));
  ASSERT_TRUE(m_domTree.dominates(*m_nodes['R'], *m_nodes['K']));
  ASSERT_TRUE(m_domTree.dominates(*m_nodes['R'], *m_nodes['L']));

  ASSERT_TRUE(m_domTree.dominates(*m_nodes['C'], *m_nodes['F']));
  ASSERT_TRUE(m_domTree.dominates(*m_nodes['C'], *m_nodes['G']));
  ASSERT_TRUE(m_domTree.dominates(*m_nodes['C'], *m_nodes['J']));

  ASSERT_TRUE(m_domTree.dominates(*m_nodes['D'], *m_nodes['L']));

  ASSERT_TRUE(m_domTree.dominates(*m_nodes['G'], *m_nodes['J']));
}

TEST_F(DominatorTreeTest, TestIDominates) {
  ASSERT_TRUE(m_domTree.iDominates(*m_nodes['R'], *m_nodes['I']));
  ASSERT_TRUE(m_domTree.iDominates(*m_nodes['R'], *m_nodes['K']));
  ASSERT_TRUE(m_domTree.iDominates(*m_nodes['R'], *m_nodes['C']));
  ASSERT_TRUE(m_domTree.iDominates(*m_nodes['R'], *m_nodes['H']));
  ASSERT_TRUE(m_domTree.iDominates(*m_nodes['R'], *m_nodes['E']));
  ASSERT_TRUE(m_domTree.iDominates(*m_nodes['R'], *m_nodes['A']));
  ASSERT_TRUE(m_domTree.iDominates(*m_nodes['R'], *m_nodes['D']));
  ASSERT_TRUE(m_domTree.iDominates(*m_nodes['R'], *m_nodes['B']));

  ASSERT_TRUE(m_domTree.iDominates(*m_nodes['C'], *m_nodes['F']));
  ASSERT_TRUE(m_domTree.iDominates(*m_nodes['C'], *m_nodes['G']));

  ASSERT_TRUE(m_domTree.iDominates(*m_nodes['D'], *m_nodes['L']));
}
