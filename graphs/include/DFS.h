#pragma once

#include <unordered_set>
#include <vector>

template <typename GraphType>
struct GraphTrait {
public:
  using NodeType = typename GraphType::NodeType;

  using SuccIterType = typename NodeType::SuccIterType;

  static SuccIterType succBegin(NodeType node);

  static SuccIterType succEnd(NodeType node);

  static NodeType getEntry(GraphType &graph);
};

template <typename GraphType, typename Trait=GraphTrait<GraphType>>
class DFSIterator {
private:
  using NodeType = typename Trait::NodeType;
  using SuccIterType = typename Trait::SuccIterType;

  struct VisitState {
    NodeType node;
    SuccIterType iter;

    bool operator==(const VisitState &rhs) const {
      return node == rhs.node && iter == rhs.iter;
    }

    bool operator!=(const VisitState &rhs) const {
      return !(*this == rhs);
    }
  };

public:
  static DFSIterator<GraphType, Trait> begin(
      GraphType &graph, std::unordered_set<NodeType> &visited) {
    return DFSIterator<GraphType, Trait>(Trait::getEntry(graph), visited);
  }

  static DFSIterator<GraphType, Trait> begin(GraphType &graph) {
    return DFSIterator<GraphType, Trait>(Trait::getEntry(graph));
  }

  static DFSIterator<GraphType, Trait> end() {
    return DFSIterator<GraphType, Trait>();
  }

  ~DFSIterator() {
    if (owningVisited)
      delete visitedNodes;
  }

  bool operator==(const DFSIterator<GraphType, Trait> &rhs) const {
    return stack == rhs.stack;
  }

  bool operator!=(const DFSIterator<GraphType, Trait> &rhs) const {
    return !(*this == rhs);
  }

  DFSIterator<GraphType, Trait> &operator++() {
    getNext();
    return *this;
  }

  DFSIterator<GraphType, Trait> operator++(int) {
    DFSIterator<GraphType, Trait> res{*this};
    getNext();
    return res;
  }

  NodeType &operator*() {
    return stack.back().node;
  }

  NodeType *operator->() {
    return &stack.back().node;
  }

private:
  DFSIterator() : stack{}, visitedNodes{nullptr}, owningVisited{false} {}

  explicit DFSIterator(NodeType root) : stack{},
      visitedNodes{new std::unordered_set<NodeType>()}, owningVisited{true} {
    init(root);
  }

  DFSIterator(NodeType root, std::unordered_set<NodeType> &visited) : stack{},
      visitedNodes{&visited}, owningVisited{false} {
    init(root);
  }

  DFSIterator(const DFSIterator<GraphType, Trait> &other): stack{other.stack},
      visitedNodes{new std::unordered_set<NodeType>(*other.visitedNodes)},
      owningVisited{true} {}

  void init(NodeType root) {
    stack.emplace_back(VisitState{root, Trait::succBegin(root)});
    visitedNodes->insert(root);
  }

  void getNext() {
    if (stack.empty())
      return;

    while (!stack.empty()) {
      auto &back = stack.back();
      auto end = Trait::succEnd(back.node);

      if (back.iter == end) {
        // All successors have been visited.
        stack.pop_back();
        continue;
      }

      if (visitedNodes->count(*back.iter)) {
        ++back.iter;
        continue;
      }

      NodeType newNode = *back.iter;
      ++back.iter;
      stack.emplace_back(VisitState{newNode, Trait::succBegin(newNode)});
      visitedNodes->insert(newNode);
      return;
    }
  }

  // If `VisitState.iter == Trait::succEnd(VisitState.node)`, then `node` and
  // all of its successors have been visited, and we need to remove such a
  // state.
  std::vector<VisitState> stack;
  std::unordered_set<NodeType> *visitedNodes;
  bool owningVisited;
};
