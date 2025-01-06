#include <gtest/gtest.h>

#include <unordered_set>

#include "DFS.h"
#include "Graph.h"

TEST(DFSTest, simpleGraph) {
  Graph graph;
  auto &nodeA = graph.createNode("node A");
  auto &nodeB = graph.createNode("node B");
  auto &nodeC = graph.createNode("node C");
  graph.connect(nodeA, nodeB);
  graph.connect(nodeB, nodeC);

  std::unordered_set<GraphNode *> visited;
  auto dfsIter = DFSIterator<Graph>::begin(graph, visited);

  ASSERT_EQ(*dfsIter, &nodeA);
  ++dfsIter;
  ASSERT_EQ(*dfsIter, &nodeB);
  ++dfsIter;
  ASSERT_EQ(*dfsIter, &nodeC);
  ++dfsIter;
  ASSERT_EQ(dfsIter, DFSIterator<Graph>::end());
}

TEST(DFSTest, cyclicGraph) {
  Graph graph;
  auto &nodeA = graph.createNode("node A");
  auto &nodeB = graph.createNode("node B");
  auto &nodeC = graph.createNode("node C");
  graph.connect(nodeA, nodeB);
  graph.connect(nodeB, nodeA);
  graph.connect(nodeB, nodeC);
  graph.connect(nodeC, nodeA);

  std::unordered_set<GraphNode *> visited;
  auto dfsIter = DFSIterator<Graph>::begin(graph, visited);

  ASSERT_EQ(*dfsIter, &nodeA);
  ++dfsIter;
  ASSERT_EQ(*dfsIter, &nodeB);
  ++dfsIter;
  ASSERT_EQ(*dfsIter, &nodeC);
  ++dfsIter;
  ASSERT_EQ(dfsIter, DFSIterator<Graph>::end());
}
