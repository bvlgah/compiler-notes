#include "Loop.h"

#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/DepthFirstIterator.h>
#include <llvm/ADT/GraphTraits.h>
#include <llvm/IR/Analysis.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/CFG.h>
#include <llvm/IR/Dominators.h>
#include <llvm/IR/PassManager.h>
#include <llvm/Support/raw_ostream.h>

#include <algorithm>
#include <cassert>
#include <list>

using namespace llvm;

namespace {

class Loop {
public:
  Loop(const BasicBlock &H, const SmallVector<const BasicBlock*> Latches,
       const DominatorTree &DomTree) {
    df_iterator_default_set<const BasicBlock*> Visited;
    Visited.insert(&H);
    addBasicBlock(H, Header);
    for (const BasicBlock *L : Latches) {
      for (const BasicBlock *BB : inverse_depth_first_ext(L, Visited)) {
        addBasicBlock(*BB);
      }
      setAttributes(*L, Latch);
    }
    // An exit of a natural loop has at least one successor that does not
    // belong to the loop.
    for (const BasicBlock *BB : BasicBlocks) {
      for (const BasicBlock *Succ : successors(BB)) {
        if (!Visited.contains(Succ)) {
          setAttributes(*BB, Exiting);
          break;
        }
      }
    }
  }

  bool isHeader(const BasicBlock &BB) const {
    return getAttributes(BB) & Header;
  }

  bool isLatch(const BasicBlock &BB) const {
    return getAttributes(BB) & Latch;
  }

  bool isExiting(const BasicBlock &BB) const {
    return getAttributes(BB) & Exiting;
  }

  bool contains(const BasicBlock &BB) const {
    return BasicBlockAttributes.contains(&BB);
  }

  using const_iterator = SmallVector<const BasicBlock*>::const_iterator;

  const_iterator begin() const { return BasicBlocks.begin(); }
  const_iterator end() const { return BasicBlocks.end(); }

private:
  enum Attributes : unsigned char {
    None = 0,
    Header = 1 << 0,
    Latch = 1 << 1,
    Exiting = 1 << 2,
  };

  void addBasicBlock(const BasicBlock &BB, Attributes Attr=None) {
    if (BasicBlockAttributes.contains(&BB))
      return;
    BasicBlocks.push_back(&BB);
    BasicBlockAttributes.insert({ &BB, Attr });
  }

  using AttributesMap = DenseMap<const BasicBlock*, Attributes>;
  using map_iterator = AttributesMap::iterator;
  using const_map_iterator = AttributesMap::const_iterator;

  map_iterator getMapIterator(const BasicBlock &BB) {
    auto IT = BasicBlockAttributes.find(&BB);
    assert(IT != BasicBlockAttributes.end()
        && "basic block not belonging to the loop");
    return IT;
  }

  const_map_iterator getMapIterator(const BasicBlock &BB) const {
    auto IT = const_cast<Loop*>(this)->getMapIterator(BB);
    return IT;
  }

  void setAttributes(const BasicBlock &BB, Attributes Attr) {
    auto IT = getMapIterator(BB);
    IT->second = static_cast<Attributes>(IT->second | Attr);
  }

  Attributes getAttributes(const BasicBlock &BB) const {
    auto IT = getMapIterator(BB);
    return IT->second;
  }

  SmallVector<const BasicBlock*> BasicBlocks;
  AttributesMap BasicBlockAttributes;
};

class LoopTreeNode {
public:
  LoopTreeNode(const Loop &L, LoopTreeNode &Parent)
      : L(L) { setParent(Parent); }

  explicit LoopTreeNode(const Loop &L)
      : L(L) {}

  using const_iterator = SmallVector<const LoopTreeNode*>::const_iterator;

  const_iterator begin() const { return Children.begin(); }
  const_iterator end() const { return Children.end(); }

  const LoopTreeNode *getParent() const { return Parent; }

  void setParent(LoopTreeNode &Parent) { setParent(&Parent); }

  void addChild(const LoopTreeNode &Child) {
    Children.push_back(&Child);
  }

  const Loop &getLoop() const { return L; }

  unsigned getDepth() const { return Depth; }

private:
  void setParent(LoopTreeNode *P) {
    resetParent();

    if (!P) return;

    assert(std::find(P->Children.begin(), P->Children.end(), this)
        == P->Children.end() && "duplicate children found");
    P->Children.push_back(this);
    Parent = P;
    Depth = P->getDepth() + 1;
  }

  void resetParent() {
    if (!Parent) return;

    auto IT = std::find(Parent->Children.begin(), Parent->Children.end(), this);
    if (IT != Parent->Children.end())
      Parent->Children.erase(IT);
    assert(std::find(Parent->Children.begin(), Parent->Children.end(), this)
        == Parent->Children.end() && "duplicate children found");
    Parent = nullptr;
    Depth = 1;
  }

  const Loop &L;
  LoopTreeNode *Parent = nullptr;
  SmallVector<const LoopTreeNode*> Children;
  unsigned Depth = 1; // 1-based
};

class LoopTree {
public:
  LoopTree() = default;

  LoopTree(const Function &F, const DominatorTree &DomTree) {
    SmallVector<LoopTreeNode*> LoopStack;
    SmallVector<const BasicBlock*> Latches;
    for (const BasicBlock *BB : depth_first(&F)) {
      for (const BasicBlock *Pred : predecessors(BB)) {
        if (DomTree.dominates(BB, Pred))
          Latches.push_back(Pred);
      }

      if (Latches.empty())
        continue;

      Loops.emplace_back(*BB, Latches, DomTree);
      // Find the outter loop
      while (!LoopStack.empty() && !LoopStack.back()->getLoop().contains(*BB))
        LoopStack.pop_back();
      if (!LoopStack.empty()) {
        LoopNodes.emplace_back(Loops.back(), *LoopStack.back());
      } else {
        LoopNodes.emplace_back(Loops.back());
        Roots.push_back(&LoopNodes.back());
      }
      LoopStack.push_back(&LoopNodes.back());
      Latches.clear();
    }
  }

  using const_iterator = SmallVector<const LoopTreeNode*>::const_iterator;

  const_iterator root_begin() const { return Roots.begin(); }
  const_iterator root_end() const { return Roots.end(); }

private:
  std::list<LoopTreeNode> LoopNodes;
  std::list<Loop> Loops;
  SmallVector<const LoopTreeNode*> Roots;
};

class LoopAnalysis : public AnalysisInfoMixin<LoopAnalysis> {
public:
  static AnalysisKey Key;

  using Result = LoopTree;

  Result run(Function &F, FunctionAnalysisManager &AM) {
    DominatorTree &DomTree = AM.getResult<DominatorTreeAnalysis>(F);
    return Result(F, DomTree);
  }
};

AnalysisKey LoopAnalysis::Key;

} // end anonymous namespace

namespace llvm {

template<>
struct GraphTraits<const LoopTreeNode*> {
  using NodeRef = const LoopTreeNode*;
  using ChildIteratorType = LoopTreeNode::const_iterator;

  static NodeRef getEntryNode(const LoopTreeNode *Node) { return Node; }

  static ChildIteratorType child_begin(NodeRef Node) { return Node->begin(); }
  static ChildIteratorType child_end(NodeRef Node) { return Node->end(); }
};

} // end namespace llvm

namespace {

class LoopPrinter : public PassInfoMixin<LoopPrinter> {
  raw_ostream &OS;

public:
  explicit LoopPrinter(raw_ostream &OS) : OS(OS) {}

  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM) {
    LoopTree &Info = AM.getResult<LoopAnalysis>(F);

    OS << "Loop info for function '" << F.getName() << "':\n";

    for (auto RootIt = Info.root_begin(), RootEnd = Info.root_end();
         RootIt != RootEnd; ++RootIt) {
      for (const LoopTreeNode *Node : depth_first(*RootIt)) {
        printLoopTreeNode(Node);
      }
    }

    return PreservedAnalyses::all();
  }

  static bool isRequired() { return true; }

private:
  void printLoopTreeNode(const LoopTreeNode *Node) {
    for (unsigned I = 1; I < Node->getDepth(); ++I)
      OS << "    ";
    OS << "Loop at depth " << Node->getDepth() << " containing: ";
    printLoop(Node->getLoop());
    OS << '\n';
  }

  void printLoop(const Loop &L) {
    auto BBIt = L.begin();
    auto BBEnd = L.end();
    if (BBIt == BBEnd)
      return;

    printBasicBlock(L, *(BBIt++));
    while (BBIt != BBEnd) {
      OS << ',';
      printBasicBlock(L, *(BBIt++));
    }
  }

  void printBasicBlock(const Loop &L, const BasicBlock *BB) {
    OS << '%' << BB->getName();
    if (L.isHeader(*BB))
      OS << "<header>";
    if (L.isLatch(*BB))
      OS << "<latch>";
    if (L.isExiting(*BB))
      OS << "<exiting>";
  }
};

} // end anonymous namespace

bool addLoopPrinter(FunctionPassManager &FPM, raw_ostream &OS) {
  FPM.addPass(LoopPrinter(OS));
  return true;
}

void addLoopAnalysis(FunctionAnalysisManager &FAM) {
  FAM.registerPass([] { return LoopAnalysis(); });
}
