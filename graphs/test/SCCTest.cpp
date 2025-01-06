#include "SCC.h"
#include "gtest/gtest.h"

#include <algorithm>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#define CONNECT_NODES(node1, node2) G.connect(*Nodes[node1], *Nodes[node2])

#define SCC_COMPUTE_AND_ASSERT()                                \
  do {                                                          \
    SCCInterfaceUP Kosaraju = createKosarajuSCC();              \
    SCCInterfaceUP Tarjan = createTarjanSCC();                  \
    ASSERT_TRUE(isEqual(Expected, Kosaraju->compute(G)));       \
    ASSERT_TRUE(isEqual(Expected, Tarjan->compute(G)));         \
  } while (0)                                                   \

bool isEqual(const SCCInterface::ResultT &Result1,
             const SCCInterface::ResultT &Result2) {
  using NodeSet = std::unordered_set<SCCInterface::SCCNode>;

  std::vector<NodeSet> SCC1, SCC2;
  for (const SCCInterface::SCCNodeVec &Nodes : Result1)
    SCC1.emplace_back(NodeSet(Nodes.begin(), Nodes.end()));
  for (const SCCInterface::SCCNodeVec &Nodes : Result2)
    SCC2.emplace_back(NodeSet(Nodes.begin(), Nodes.end()));

  if (SCC1.size() != SCC2.size())
    return false;
  for (const NodeSet &Nodes : SCC1) {
    if (std::find(SCC2.begin(), SCC2.end(), Nodes) == SCC2.end())
      return false;
  }
  return true;
}

TEST(SCCTest, TestCase1) {
  Graph G;
  std::unordered_map<char, GraphNode *> Nodes;
  for (char Start = 'A'; Start <= 'I'; ++Start)
    Nodes.emplace(Start, &G.createNode(std::string(1, Start).c_str()));

  // This test case is from https://www.cs.cornell.edu/courses/cs4120/2023sp/notes.html?id=iterative
  CONNECT_NODES('A', 'B');
  CONNECT_NODES('A', 'D');
  CONNECT_NODES('B', 'C');
  CONNECT_NODES('C', 'B');
  CONNECT_NODES('C', 'H');
  CONNECT_NODES('H', 'I');
  CONNECT_NODES('D', 'E');
  CONNECT_NODES('E', 'F');
  CONNECT_NODES('F', 'E');
  CONNECT_NODES('F', 'G');
  CONNECT_NODES('G', 'E');
  CONNECT_NODES('G', 'H');

  SCCInterface::ResultT Expected {{
    { Nodes['A'] },
    { Nodes['D'] },
    { Nodes['E'], Nodes['F'], Nodes['G'] },
    { Nodes['B'], Nodes['C'] },
    { Nodes['H'] },
    { Nodes['I'] },
  }};
  SCC_COMPUTE_AND_ASSERT();
}

TEST(SCCTest, TestCase2) {
  Graph G;
  std::unordered_map<char, GraphNode *> Nodes;
  for (char Start = 'a'; Start <= 'h'; ++Start)
    Nodes.emplace(Start, &G.createNode(std::string(1, Start).c_str()));

  // This test case is from Introduction to Algorithms 4th, 20.5 Strongly
  // connected components.
  CONNECT_NODES('a', 'b');
  CONNECT_NODES('b', 'c');
  CONNECT_NODES('b', 'e');
  CONNECT_NODES('b', 'f');
  CONNECT_NODES('c', 'd');
  CONNECT_NODES('c', 'g');
  CONNECT_NODES('d', 'c');
  CONNECT_NODES('d', 'h');
  CONNECT_NODES('e', 'a');
  CONNECT_NODES('e', 'f');
  CONNECT_NODES('f', 'g');
  CONNECT_NODES('g', 'f');
  CONNECT_NODES('g', 'h');
  CONNECT_NODES('h', 'h');

  SCCInterface::ResultT Expected {{
    { Nodes['a'], Nodes['b'], Nodes['e'] },
    { Nodes['c'], Nodes['d'] },
    { Nodes['f'], Nodes['g'] },
    { Nodes['h'] },
  }};
  SCC_COMPUTE_AND_ASSERT();
}
