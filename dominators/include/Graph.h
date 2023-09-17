#pragma once

#include <memory>
#include <ostream>
#include <string>
#include <vector>

class Graph;

class GraphNode;

using GraphNodeWP = std::weak_ptr<GraphNode>;

using GraphNodeSP = std::shared_ptr<GraphNode>;

int printPtr(char *buf, std::size_t bufSize, const void *ptr);

class GraphNode {
public:
    using const_iterator = std::vector<GraphNodeWP>::const_iterator;
    using const_reverse_iterator =
        std::vector<GraphNodeWP>::const_reverse_iterator;

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

private:
    friend class Graph;

    explicit GraphNode(const char *name, Graph &parent)
        : m_name(name), m_predecessors(), m_successors(), m_parent(parent) {}

    GraphNode(const GraphNode &other) = delete;

    GraphNode &operator=(const GraphNode &other) = delete;

    bool AddPredecessor(const GraphNodeWP &pred);

    bool AddSuccessor(const GraphNodeWP &succ);

    std::string m_name;
    std::vector<GraphNodeWP> m_predecessors;
    std::vector<GraphNodeWP> m_successors;
    Graph &m_parent;
};

class Graph {
public:
    using const_iterator = std::vector<GraphNodeSP>::const_iterator;

    Graph() : m_nodes() {};

    GraphNodeSP createNode(const char *name);

    GraphNodeSP createNode(std::nullptr_t) = delete;

    bool connect(GraphNodeSP &a, GraphNodeSP &b);

    const_iterator begin() const { return m_nodes.begin(); }

    const_iterator end() const { return m_nodes.end(); }

    void print(std::ostream &output) const;

private:
    std::vector<GraphNodeSP> m_nodes;
};
