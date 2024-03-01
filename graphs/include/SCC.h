#pragma once

#include "Graph.h"

#include <memory>
#include <vector>

class SCCInterface {
public:
  using SCCNode = GraphNodeSP;
  using SCCNodeVec = std::vector<SCCNode>;
  using ResultT = std::vector<SCCNodeVec>;

  virtual ~SCCInterface() = default;

  virtual std::vector<SCCNodeVec> compute(const Graph &G) = 0;
};

using SCCInterfaceUP = std::unique_ptr<SCCInterface>;

SCCInterfaceUP createKosarajuSCC();

SCCInterfaceUP createTarjanSCC();
