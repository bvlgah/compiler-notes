#pragma once

#include <algorithm>
#include <cassert>
#include <memory>
#include <ostream>
#include <unordered_map>
#include <vector>

#include "Graph.h"

class DominatorTree {
public:
    DominatorTree() : m_nodeMap(), m_root() {}

    bool reconstruct(const Graph &graph, const GraphNodeSP &entry);

    bool dominates(const GraphNodeSP &node1, const GraphNodeSP &node2) const;

    bool iDominates(const GraphNodeSP &parent, const GraphNodeSP &child) const;

    void print(std::ostream &output) const;

private:
    class DominatorTreeNode;

    using DominatorTreeNodeSP = std::shared_ptr<DominatorTreeNode>;
    using DominatorTreeNodeWP = std::weak_ptr<DominatorTreeNode>;

    class DominatorTreeNode
        : public std::enable_shared_from_this<DominatorTreeNode> {
    public:
        using const_iterator = std::vector<DominatorTreeNodeSP>::const_iterator;

        static constexpr int ROOT_LEVEL = 0;

        DominatorTreeNode(const GraphNodeSP &graphNode, int level)
            : m_parent(), m_children(),
              m_level(level), m_graphNode(graphNode) {}

        const_iterator begin() const { return m_children.begin(); }

        const_iterator end() const { return m_children.end(); }

        int getLevel() const { return m_level; }

        const GraphNodeWP &getGraphNode() const { return m_graphNode; }

        void setParent(const DominatorTreeNodeWP &parent) {
            DominatorTreeNodeSP oldParent = m_parent.lock();
            DominatorTreeNodeSP newParent = parent.lock();

            if (oldParent == newParent) {
                return;
            }

            if (oldParent) {
                const_iterator it = std::find(
                    oldParent->begin(), oldParent->end(), shared_from_this());
                assert(it != oldParent->end() &&
                    "oldParent should has this node as child");
                oldParent->m_children.erase(it);
            }
            if (newParent) {
                newParent->m_children.push_back(shared_from_this());
            }
            m_parent = newParent;
        }

        const DominatorTreeNodeWP &getParent() const { return m_parent; }

    private:
        DominatorTreeNodeWP m_parent;
        std::vector<DominatorTreeNodeSP> m_children;
        GraphNodeWP m_graphNode;
        int m_level;
    };

    constexpr static int SLT_FOREST_INVALID_DFS_NUM = -1;
    constexpr static int SLT_FOREST_INIT_DFS_NUM = -1;

    struct SLTForestNode {
        int parent = SLT_FOREST_INVALID_DFS_NUM;
        int dfsNum = SLT_FOREST_INVALID_DFS_NUM;
        int semiDom = SLT_FOREST_INVALID_DFS_NUM;
        int iDom = SLT_FOREST_INVALID_DFS_NUM;
        int label = SLT_FOREST_INVALID_DFS_NUM;
        int ancestor = SLT_FOREST_INVALID_DFS_NUM;
        GraphNodeSP graphNode;
        std::vector<int> bucket; // which this node is the semi-domniator of

        SLTForestNode(int parent, int dfsNum, const GraphNodeSP &graphNodeSP)
            : parent(parent), dfsNum(dfsNum), semiDom(dfsNum), iDom(dfsNum),
              label(dfsNum), graphNode(graphNodeSP),
              bucket() {}

        void print(std::ostream &ouput) const;
    };

    using SLTForestNodeSP = std::shared_ptr<SLTForestNode>;

    class SLTForest {
    public:
        SLTForest();

        const std::vector<SLTForestNodeSP> &recalculate(
            const GraphNodeSP &entry);

        void print(std::ostream &ouput) const;

    private:
        void DoDFS(const GraphNodeSP &entry);

        SLTForestNodeSP CreateForestNode(int parent, const GraphNodeSP &child);

        SLTForestNodeSP FindForestNode(const GraphNodeSP &graphNodeSP) const;

        void Clear();

        void Link(int node);

        int Eval(int node);

        bool FindNextUnvisited(
            GraphNode::const_iterator &it,
            const GraphNode::const_iterator &end);

        std::vector<SLTForestNodeSP> m_forestNodes;
        std::unordered_map<GraphNodeSP, SLTForestNodeSP> m_nodeMap;
        int m_lastDFSNum;
    };

    std::unordered_map<GraphNodeSP, DominatorTreeNodeSP> m_nodeMap;
    DominatorTreeNodeSP m_root;
};
