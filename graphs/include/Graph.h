#pragma once

#include "DFS.h"

#include <list>
#include <ostream>
#include <string>
#include <vector>

class Graph;

class GraphNode;

int printPtr(char *buf, std::size_t bufSize, const void *ptr);

class GraphNode {
public:
    using const_iterator = std::vector<GraphNode *>::const_iterator;
    using const_reverse_iterator =
        std::vector<GraphNode *>::const_reverse_iterator;

    const_iterator succBegin() const { return m_successors.cbegin(); }

    const_iterator succEnd() const { return m_successors.cend(); }

    const_iterator predBegin() const { return m_predecessors.cbegin(); }

    const_iterator predEnd() const { return m_predecessors.cend(); }

    const_reverse_iterator succRBegin() const {
      return m_predecessors.crbegin();
    }

    const_reverse_iterator succREnd() const {
      return m_predecessors.crend();
    }

    Graph &getParent() { return m_parent; }

    const std::string &getName() const { return m_name; }

    void print(std::ostream &output) const;

    GraphNode(const char *name, Graph &parent)
        : m_name(name), m_predecessors(), m_successors(), m_parent(parent) {}

private:
    friend class Graph;

    GraphNode(const GraphNode &other) = delete;

    GraphNode &operator=(const GraphNode &other) = delete;

    bool AddPredecessor(GraphNode &pred);

    bool AddSuccessor(GraphNode &succ);

    std::string m_name;
    std::vector<GraphNode *> m_predecessors;
    std::vector<GraphNode *> m_successors;
    Graph &m_parent;
};

class Graph {
private:
    using Container = std::list<GraphNode>;

public:
    using const_iterator = Container::const_iterator;
    using iterator = Container::iterator;

    Graph() : m_nodes() {};

    GraphNode &createNode(const char *name);

    GraphNode &createNode(std::nullptr_t) = delete;

    bool connect(GraphNode &a, GraphNode &b);

    const_iterator begin() const { return m_nodes.begin(); }

    const_iterator end() const { return m_nodes.end(); }

    iterator begin() { return m_nodes.begin(); }

    iterator end() { return m_nodes.end(); }

    void print(std::ostream &output) const;

private:
    Container m_nodes;
};

template <>
struct GraphTrait<Graph> {
  using NodeType = GraphNode *;

  using SuccIterType = GraphNode::const_iterator;

  static SuccIterType succBegin(NodeType node) {
    return node->succBegin();
  }

  static SuccIterType succEnd(NodeType node) {
    return node->succEnd();
  }

  static NodeType getEntry(Graph &graph) {
    return &(*graph.begin());
  }
};
