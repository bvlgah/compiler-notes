// In order to use the macro PRIxPTR
// #define __STDC_FORMAT_MACROS
#include <algorithm>
#include <cassert>
#include <cinttypes>
#include <cstdio>

#include "Graph.h"

static bool operator==(const GraphNodeWP &a, const GraphNodeWP &b) {
    return a.lock() == b.lock();
}

bool GraphNode::AddPredecessor(const GraphNodeWP &pred) {
    assert(!pred.expired() && "pred should point to an existing graph node");
    const auto it = std::find(
        m_predecessors.begin(), m_predecessors.end(), pred);
    if (it != m_predecessors.end()) {
        return false;
    } else {
        m_predecessors.push_back(pred);
        return true;
    }
}

bool GraphNode::AddSuccessor(const GraphNodeWP &succ) {
    assert(!succ.expired() && "succ should point to an existing graph node");
    const auto it = std::find(
        m_successors.begin(), m_successors.end(), succ);
    if (it != m_successors.end()) {
        return false;
    } else {
        m_successors.push_back(succ);
        return true;
    }
}

GraphNodeSP Graph::createNode(const char *name) {
    assert(name && "name should be a non-nullptr c-string");
    GraphNodeSP node(new GraphNode(name, *this));
    m_nodes.push_back(node);
    return node;
}

bool Graph::connect(GraphNodeSP &a, GraphNodeSP &b) {
    assert(a.get() && "a should point to an existing graph node");
    assert(b.get() && "b should point to an existing graph node");
    return a->AddSuccessor(b) && b->AddPredecessor(a);
}

int printPtr(char *buf, std::size_t bufSize, const void *ptr) {
    return std::snprintf(buf, bufSize, "0x%" PRIxPTR,
        reinterpret_cast<std::uintptr_t>(ptr));
}

static void PrintGraphNode(std::ostream &output, const GraphNode &node) {
    char ptrFmtBuf[20];
    constexpr std::size_t ptrFmtBufSize = sizeof(ptrFmtBuf);
    printPtr(ptrFmtBuf, ptrFmtBufSize, &node);
    output << "<GraphNode \"" << node.getName() << "\" " << ptrFmtBuf << ">";
}

static void PrintGraphNodeList(std::ostream &output,
    GraphNode::const_iterator begin, GraphNode::const_iterator end) {
    output << "[";

    if (begin == end) {
        output << "]";
        return;
    }

    PrintGraphNode(output, *(begin->lock()));
    begin++;

    while (begin != end) {
        output << ", ";
        PrintGraphNode(output, *(begin->lock()));
        begin++;
    }

    output << "]";
}

void GraphNode::print(std::ostream &output) const {
    PrintGraphNode(output, *this);
}

void Graph::print(std::ostream &output) const {
    assert(sizeof(this) <= 8 &&
        "size of a pointer should be less than or equal to 8");
    // A buffer of size 20 should be long enough to hold a formatted string for
    // a pointer of size 4 or 8, e.g. an 8-byte pointer with value
    // 0x0123456789abcdef.
    char ptrFmtBuf[20];
    constexpr std::size_t ptrFmtBufSize = sizeof(ptrFmtBuf);
    printPtr(ptrFmtBuf, ptrFmtBufSize, this);
    output << "<Graph " << ptrFmtBuf << ">" << std::endl;
    constexpr const char *nodeIndent = "    ";
    constexpr const char *predIndent = "        ";
    constexpr const char *succIndent = predIndent;

    for (const GraphNodeSP &nodeSP : *this) {
        output << nodeIndent;
        PrintGraphNode(output, *nodeSP);
        output << std::endl << predIndent << "predecessors: ";
        PrintGraphNodeList(output, nodeSP->predBegin(), nodeSP->predEnd());
        output << std::endl << succIndent << "successors: ";
        PrintGraphNodeList(output, nodeSP->succBegin(), nodeSP->succEnd());
        output << std::endl;
    }
}
