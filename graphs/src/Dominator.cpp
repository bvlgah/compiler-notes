#include <algorithm>
#include <cinttypes>
#include <iostream>
#include <string>
#include <utility>

#include "Dominator.h"

void DominatorTree::SLTForestNode::print(std::ostream &ouput) const {
    graphNode.print(std::cout);
    std::cout << ": dfsNum " << dfsNum <<
                 ", iDom " << iDom <<
                 ", semiDom " << semiDom <<
                 ", parent " << parent <<
                 ", ancestor " << ancestor <<
                 ", label " << label;
    std::cout << std::endl;
}

const std::vector<DominatorTree::SLTForestNodeSP> &
DominatorTree::SLTForest::recalculate(const GraphNode &entry) {
    Clear();
    DoDFS(entry);

    std::size_t nodeSize = m_forestNodes.size();
    assert(static_cast<int>(nodeSize - 1) == m_lastDFSNum &&
        "m_lastDFSNum is not right");
    for (int node = static_cast<int>(nodeSize) - 1; node > 0; node--) {
        SLTForestNodeSP &forestNode = m_forestNodes[node];
        assert(forestNode.get() && "forestNode can not be nullptr");

        /* std::cout << __func__ << */
        /*              " stage 1: before calculating semi-dominators for "; */
        /* forestNode->print(std::cout); */
        for (auto predIt = forestNode->graphNode.predBegin();
             predIt != forestNode->graphNode.predEnd();
             predIt++) {
             const SLTForestNodeSP &predNode = m_nodeMap[*predIt];
             /* std::cout << "    predecessor: "; */
             /* predNode->print(std::cout); */
             int minLabel = Eval(predNode->dfsNum);
             /* std::cout << "    minLabel = " << minLabel << std::endl; */
             forestNode->semiDom = std::min(
                 m_forestNodes[minLabel]->semiDom, forestNode->semiDom);
        }
        /* std::cout << __func__ << */
        /*              " stage 1: after calculating semi-dominators for "; */
        /* forestNode->print(std::cout); */

        int semiDom = forestNode->semiDom;
        SLTForestNodeSP &semiDomNode = m_forestNodes[semiDom];
        assert(semiDomNode.get() && "semiDomNode can not be nullptr");
        semiDomNode->bucket.push_back(forestNode->dfsNum);

        Link(node);
        int parent = forestNode->parent;
        SLTForestNodeSP &parentNode = m_forestNodes.at(parent);
        for (int n : parentNode->bucket) {
            /* std::cout << "stage 2: node " << n << " in bucket of node " << */
            /*     forestNode->dfsNum; */
            int minLabel = Eval(n);
            /* std::cout << ", minLabel = " << minLabel << std::endl; */
            m_forestNodes[n]->iDom =
                (parentNode->dfsNum == m_forestNodes[minLabel]->semiDom) ?
                parentNode->dfsNum : minLabel;
        }
    }

    for (int node = 1; node < static_cast<int>(nodeSize); node++) {
        SLTForestNodeSP &forestNode = m_forestNodes[node];
        if (forestNode->semiDom != forestNode->iDom) {
            int iDom = forestNode->iDom;
            forestNode->iDom = m_forestNodes[iDom]->iDom;
        }
    }

    return m_forestNodes;
}

void DominatorTree::SLTForest::DoDFS(const GraphNode &entry) {
    assert(m_lastDFSNum == SLT_FOREST_INIT_DFS_NUM &&
        m_forestNodes.empty() && m_nodeMap.empty() &&
        "can not run DoDFS on an already constructed forest");

    using GraphNodeAndIt = std::pair<const GraphNode &, GraphNode::const_iterator>;
    std::vector<GraphNodeAndIt> graphNodeStack;
    graphNodeStack.emplace_back(entry, entry.succBegin());
    CreateForestNode(SLT_FOREST_INVALID_DFS_NUM, entry);

    while (!graphNodeStack.empty()) {
        auto &[graphNode, succIt] = graphNodeStack.back();

        if (succIt == graphNode.succEnd()) {
            graphNodeStack.pop_back();
            continue;
        }

        if (FindNextUnvisited(succIt, graphNode.succEnd())) {
            SLTForestNodeSP forestNodeSP = CreateForestNode(
                m_nodeMap.at(&graphNode)->dfsNum, **succIt);
            const GraphNode *succ = *succIt;
            graphNodeStack.emplace_back(*succ, succ->succBegin());
        }
    }
}

bool DominatorTree::SLTForest::FindNextUnvisited(
    GraphNode::const_iterator &it,
    const GraphNode::const_iterator &end) {
    while (it != end) {
        if (m_nodeMap.count(*it) == 0) {
            return true;
        }
        it++;
    }
    return false;
}

DominatorTree::SLTForestNodeSP DominatorTree::SLTForest::CreateForestNode(
    int parent, const GraphNode &child) {
    assert(m_nodeMap.find(&child) == m_nodeMap.end() &&
        "child is expected not visited");
    SLTForestNodeSP forestNodeSP = std::make_shared<SLTForestNode>(
        parent, ++m_lastDFSNum, child);
    m_nodeMap.emplace(&child, forestNodeSP);
    m_forestNodes.emplace_back(forestNodeSP);
    return forestNodeSP;
}

DominatorTree::SLTForestNodeSP DominatorTree::SLTForest::FindForestNode(
    const GraphNode &graphNode) const {
    const auto it = m_nodeMap.find(&graphNode);
    if (it == m_nodeMap.end()) {
        return SLTForestNodeSP();
    } else {
        return it->second;
    }
}

DominatorTree::SLTForest::SLTForest()
    : m_forestNodes(), m_lastDFSNum(), m_nodeMap() {
    Clear();
}

void DominatorTree::SLTForest::Clear() {
    m_forestNodes.clear();
    m_nodeMap.clear();
    m_lastDFSNum = SLT_FOREST_INIT_DFS_NUM;
}

void DominatorTree::SLTForest::print(std::ostream &output) const {
    constexpr const char *indent = "  ";
    char ptrStr[20];

    printPtr(ptrStr, sizeof(ptrStr), this);
    output << "<SLTForest " << ptrStr << ">" << std::endl;
    for (const auto &forestNode : m_forestNodes) {
        output << indent << forestNode->dfsNum << ": ";
        forestNode->graphNode.print(output);
        output << std::endl;
    }
}

void DominatorTree::SLTForest::Link(int nodeNum) {
    assert(nodeNum <= m_lastDFSNum &&
        "nodeNum should be less than m_lastDFSNum");
    assert(m_forestNodes[nodeNum]->ancestor == SLT_FOREST_INIT_DFS_NUM &&
        "re-link a linked node");
    m_forestNodes[nodeNum]->ancestor = m_forestNodes[nodeNum]->parent;
}

int DominatorTree::SLTForest::Eval(int nodeNum) {
    assert(nodeNum <= m_lastDFSNum &&
        "nodeNum should be less than m_lastDFSNum");
    if (m_forestNodes[nodeNum]->ancestor == SLT_FOREST_INIT_DFS_NUM) {
        return nodeNum;
    }

    std::vector<int> nodesWithAncestor {};
    for (int node = nodeNum, ancestor = m_forestNodes[node]->ancestor;
         m_forestNodes[ancestor]->ancestor != SLT_FOREST_INIT_DFS_NUM;) {
        /* std::cout << __func__ << " node = " << node << */
        /*     ", ancestor = " << ancestor << std::endl; */
        nodesWithAncestor.emplace_back(node);
        int newNode = m_forestNodes[node]->ancestor;
        int newAncestor = m_forestNodes[ancestor]->ancestor;
        node = newNode;
        ancestor = newAncestor;
    }
    while (!nodesWithAncestor.empty()) {
        int node = nodesWithAncestor.back();
        int ancestor = m_forestNodes[node]->ancestor;
        int label = m_forestNodes[node]->label;
        int ancestorLabel = m_forestNodes[ancestor]->label;
        if (m_forestNodes[ancestorLabel]->semiDom <
            m_forestNodes[label]->semiDom) {
            m_forestNodes[node]->label = ancestorLabel;
        }
        m_forestNodes[node]->ancestor = m_forestNodes[ancestor]->ancestor;
        nodesWithAncestor.pop_back();
    }

    return m_forestNodes[nodeNum]->label;
}

bool DominatorTree::reconstruct(const Graph &graph, const GraphNode &entry) {
    SLTForest forest {};
    const auto forestNodes = forest.recalculate(entry);
    assert(!forestNodes.empty() && "forest should not be empty");

    /* for (const auto &forestNode : forestNodes) { */
    /*     forestNode->print(std::cout); */
    /* } */

    m_root = std::make_shared<DominatorTreeNode>(
        forestNodes[0]->graphNode, DominatorTreeNode::ROOT_LEVEL);
    m_nodeMap.emplace(&m_root->getGraphNode(), m_root);

    for (int i = 1; i < static_cast<int>(forestNodes.size()); i++) {
        const SLTForestNodeSP &node = forestNodes[i];
        const GraphNode &graphNode = node->graphNode;
        int iDom = node->iDom;
        const SLTForestNodeSP &iDomForestNode = forestNodes[iDom];
        const GraphNode &iDomGraphNode = iDomForestNode->graphNode;
        const DominatorTreeNodeSP &iDomTreeNode = m_nodeMap[&iDomGraphNode];
        DominatorTreeNodeSP treeNode = std::make_shared<DominatorTreeNode>(
            graphNode, iDomTreeNode->getLevel() + 1);
        treeNode->setParent(iDomTreeNode);
        m_nodeMap.emplace(&graphNode, treeNode);
    }

    return false;
}

bool DominatorTree::dominates(
    const GraphNode &node1, const GraphNode &node2) const {
    const auto &treeNodeIt1 = m_nodeMap.find(&node1);
    const auto &treeNodeIt2 = m_nodeMap.find(&node2);
    assert(treeNodeIt1 != m_nodeMap.end() && treeNodeIt1 != m_nodeMap.end() &&
        "node1 and node2 should be in the dominator tree");
    const DominatorTreeNodeSP &treeNode1 = treeNodeIt1->second;
    const DominatorTreeNodeSP &treeNode2 = treeNodeIt2->second;

    if (treeNode1->getLevel() == treeNode2->getLevel()) {
        return treeNode1 == treeNode2;
    } else if (treeNode1->getLevel() > treeNode2->getLevel()) {
        return false;
    }

    DominatorTreeNodeSP ancestor = treeNode2->getParent().lock();
    while (ancestor->getLevel() != treeNode1->getLevel()) {
        ancestor = ancestor->getParent().lock();
    }

    return treeNode1 == ancestor;
}

bool DominatorTree::iDominates(
    const GraphNode &parent, const GraphNode &child) const {
  const auto &parentIt = m_nodeMap.find(&parent);
  const auto &childIt = m_nodeMap.find(&child);
  assert(parentIt != m_nodeMap.end() &&
         childIt != m_nodeMap.end() &&
         "parent and child should be in the dominator tree");

  const auto &parentNode = parentIt->second;
  const auto &childNode = childIt->second;
  if (parentNode->getLevel() + 1 == childNode->getLevel()) {
    return childNode->getParent().lock() == parentNode;
  } else {
    return false;
  }
}

static void PrintGraphNode(
    int level, const GraphNode &graphNode, std::ostream &output) {
    constexpr int nodeIndent = 4;
    output << std::string(static_cast<std::size_t>(nodeIndent * level), ' ');
    graphNode.print(output);
    output << std::endl;
}

void DominatorTree::print(std::ostream &output) const {
    using TreeNodeAndIt = std::pair<
        DominatorTreeNodeSP, DominatorTreeNode::const_iterator>;

    std::vector<TreeNodeAndIt> nodeStack {{{ m_root, m_root->begin() }}};
    PrintGraphNode(m_root->getLevel(), m_root->getGraphNode(), output);

    while (!nodeStack.empty()) {
        auto &[treeNode, childIt] = nodeStack.back();
        if (childIt == treeNode->end()) {
            nodeStack.pop_back();
            continue;
        }
        const DominatorTreeNodeSP &child = *childIt;
        childIt++;
        PrintGraphNode(child->getLevel(), child->getGraphNode(), output);
        nodeStack.emplace_back(child, child->begin());
    }
    output << "root: ";
    PrintGraphNode(m_root->getLevel(), m_root->getGraphNode(), output);
}
