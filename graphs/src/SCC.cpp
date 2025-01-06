#include "SCC.h"
#include "Utils.h"

#include <algorithm>
#include <cassert>
#include <deque>
#include <stdexcept>
#include <unordered_set>
#include <unordered_map>
#include <utility>
#include <vector>

namespace {

class ForwardVisitState {
  SCCInterface::SCCNode Node;
  GraphNode::const_iterator Current;

public:
  explicit ForwardVisitState(SCCInterface::SCCNode Node)
    : Node(Node), Current(Node->succBegin()) {}

  bool isAllChildrenVisited() const {
    return Current == Node->succEnd();
  }

  SCCInterface::SCCNode nextChild() {
    assert(!isAllChildrenVisited() && "all successors are visited");
    SCCInterface::SCCNode Child = *Current;
    ++Current;
    return Child;
  }

  SCCInterface::SCCNode getNode() const {
    return Node;
  }
};

// Fixme: Use template to merge ForwardVisitState and BackwardVisitState
// together.
class BackwardVisitState {
  SCCInterface::SCCNode Node;
  GraphNode::const_iterator Current;

public:
  explicit BackwardVisitState(SCCInterface::SCCNode Node)
    : Node(Node), Current(Node->predBegin()) {}

  bool isAllChildrenVisited() const {
    return Current == Node->predEnd();
  }

  SCCInterface::SCCNode nextChild() {
    assert(!isAllChildrenVisited() && "all predecessors are visited");
    SCCInterface::SCCNode Child = *Current;
    ++Current;
    return Child;
  }

  SCCInterface::SCCNode getNode() const {
    return Node;
  }
};

class SCCKosaraju : public SCCInterface {
public:
  ResultT compute(const Graph &G) override {
    std::deque<SCCNode> PostOrder;
    {
      std::vector<ForwardVisitState> VisitStack;
      std::unordered_set<SCCNode> Discovered;
      VisitStack.emplace_back(&(*G.begin()));
      Discovered.emplace(&(*G.begin()));

      while (!VisitStack.empty()) {
        // invariant: nodes on the stack are not visited yet
        while (!VisitStack.back().isAllChildrenVisited()) {
          SCCNode Child = VisitStack.back().nextChild();
          if (!Discovered.count(Child)) {
            VisitStack.emplace_back(Child);
            Discovered.emplace(Child);
          }
        }
        PostOrder.emplace_back(VisitStack.back().getNode());
        VisitStack.pop_back();
      }
    }

    ResultT Result;
    {
      std::vector<BackwardVisitState> VisitStack;
      std::unordered_set<SCCNode> Visited;
      for (auto IT = PostOrder.rbegin(), End = PostOrder.rend();
           IT != End; ++IT) {
        if (Visited.count(*IT))
          continue;

        SCCNodeVec Nodes { *IT };
        VisitStack.emplace_back(*IT);
        Visited.emplace(*IT);

        while (!VisitStack.empty()) {
          while (!VisitStack.back().isAllChildrenVisited()) {
            SCCNode Child = VisitStack.back().nextChild();
            if (!Visited.count(Child)) {
              Nodes.emplace_back(Child);
              VisitStack.emplace_back(Child);
              Visited.emplace(Child);
            }
          }

          VisitStack.pop_back();
        }

        Result.emplace_back(Nodes);
      }
    }

    return Result;
  }

private:
};

class SCCTarjan : public SCCInterface {
public:
  ResultT compute(const Graph &G) override {
    ResultT Result;
    unsigned DFSNumber = 0;
    std::vector<ForwardVisitState> VisitStack;
    std::unordered_set<SCCNode> Visited;
    std::unordered_map<SCCNode, DFSState> SCCStates;
    std::vector<SCCNode> SCCCandiates;

    VisitStack.emplace_back(&(*G.begin()));
    Visited.emplace(&(*G.begin()));
    SCCCandiates.emplace_back(&(*G.begin()));
    SCCStates.emplace(&(*G.begin()), DFSState(DFSNumber++));

    while (!VisitStack.empty()) {
      while (!VisitStack.back().isAllChildrenVisited()) {
        auto &CurrVisitState = VisitStack.back();
        DEBUG_MESSAGE("node on VisitStack: " +
                     CurrVisitState.getNode()->getName());
        SCCNode Child = CurrVisitState.nextChild();
        DEBUG_MESSAGE("child: " + Child->getName());
        if (Visited.count(Child)) {
          auto It = SCCStates.find(Child);
          if (It != SCCStates.end()) {
            DEBUG_MESSAGE("node visited before and on the same subtree: " +
                          Child->getName());
            assert(SCCStates.count(CurrVisitState.getNode()) &&
                   "current node is not in SCCStates");
            SCCStates.at(CurrVisitState.getNode()).updateLowest(
                It->second.getLowest());
          }
        } else {
          SCCCandiates.emplace_back(Child);
          SCCStates.emplace(Child, DFSState(DFSNumber++));
          Visited.emplace(Child);
          VisitStack.emplace_back(Child);
        }
      }

      SCCNode CurrNode = VisitStack.back().getNode();
      VisitStack.pop_back();
      assert(SCCStates.count(CurrNode) && "current node is not in SCCStates");
      DFSState CurrState = SCCStates.at(CurrNode);
      if (CurrState.getDFSNumber() == CurrState.getLowest()) {
        DEBUG_MESSAGE("node to pop from VisitStack: " + CurrNode->getName());
        SCCNodeVec SCCNodes;
        SCCNode Node = SCCCandiates.back();
        while (Node != CurrNode) {
          DEBUG_MESSAGE("SCC node: " + Node->getName());
          SCCNodes.emplace_back(Node);
          SCCCandiates.pop_back();
          assert(SCCStates.count(Node) && "node is not in SCCStates");
          SCCStates.erase(SCCStates.find(Node));
          Node = SCCCandiates.back();
        }
        DEBUG_MESSAGE("SCC node: " + Node->getName());
        SCCNodes.emplace_back(Node);
        SCCCandiates.pop_back();
        SCCStates.erase(SCCStates.find(Node));
        Result.emplace_back(SCCNodes);
      } else {
        assert(!VisitStack.empty() && "root of SCC is not on VisitStack");
        SCCNode Parent = VisitStack.back().getNode();
        SCCStates.at(Parent).updateLowest(SCCStates.at(CurrNode).getLowest());
      }
    }

    return Result;
  }

private:
  struct DFSState {
  private:
    unsigned DFSNumber;
    unsigned Lowest;

  public:
    explicit DFSState(unsigned Number) : DFSNumber(Number), Lowest(Number) {}

    unsigned getDFSNumber() const { return DFSNumber; }

    unsigned getLowest() const { return Lowest; }

    void updateLowest(unsigned Number) { Lowest = std::min(Lowest, Number); }
  };
};

} // end anonymous namespace

SCCInterfaceUP createKosarajuSCC() {
  return std::make_unique<SCCKosaraju>();
}

SCCInterfaceUP createTarjanSCC() {
  return std::make_unique<SCCTarjan>();
}
