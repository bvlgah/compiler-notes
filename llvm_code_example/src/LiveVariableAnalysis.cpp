#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/Pass.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/raw_ostream.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <queue>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

using namespace llvm;

namespace {

class Analyzer {
public:
  explicit Analyzer(const Function &F)
      : Fun(F), Blocks(), LiveIns(), LiveOuts(), Defs(), Uses() {
    initVisitingOrder();
  }

  void run() {
    init();
    compute();
    print();
  }

private:
  using ValueSetT = std::unordered_set<const Value *>;
  using BlockValueMapT = std::unordered_map<const BasicBlock *, ValueSetT>;

  void initVisitingOrder() {
    Blocks.reserve(Fun.size());
    SmallPtrSet<const BasicBlock *, 8> VisitedBB;
    for (const BasicBlock &BB : Fun) {
      if (!succ_empty(&BB))
        continue;
      std::copy(ipo_ext_begin<const BasicBlock*>(&BB, VisitedBB),
                ipo_ext_end<const BasicBlock *>(&BB, VisitedBB),
                std::back_inserter(Blocks));
    }
    assert(Blocks.size() == Fun.size() && "some basic blocks are not visited");
  }

  void init() {
    for (const BasicBlock *Block : Blocks) {
      LiveIns.emplace(Block, ValueSetT());
      LiveOuts.emplace(Block, ValueSetT());
      BlockValueMapT::iterator DefIt;
      BlockValueMapT::iterator UseIt;
      std::tie(DefIt, std::ignore) = Defs.emplace(Block, ValueSetT());
      std::tie(UseIt, std::ignore) = Uses.emplace(Block, ValueSetT());

      for (const Instruction &Inst : *Block) {
        for (const Use &U : Inst.operands()) {
          const Value *V = U.get();
          if (llvm::isa<llvm::Constant>(V)
              || V->getType()->isLabelTy()
              || DefIt->second.count(V))
            continue;
          UseIt->second.insert(V);
        }
        if (!Inst.getType()->isVoidTy() && !UseIt->second.count(&Inst))
          DefIt->second.insert(&Inst);
      }
    }
  }

  void print() const {
    llvm::errs() << "Result of live variable analyses for function '"
                 << Fun.getName() << "'\n";
    for (const BasicBlock &Block : Fun) {
      llvm::errs() << "  live-in variables of block '"
                   << Block.getName() << "': ";
      auto LiveInIt = LiveIns.find(&Block);
      assert(LiveInIt != LiveIns.end() && "an unknown basic block");
      printValueSet(llvm::errs(), LiveInIt->second);
      llvm::errs() << "\n";

      auto LiveOutIt = LiveOuts.find(&Block);
      assert(LiveOutIt != LiveOuts.end() && "an unknown basic block");
      llvm::errs() << "  live-out variables of block '"
                   << Block.getName() << "': ";
      printValueSet(llvm::errs(), LiveOutIt->second);
      llvm::errs() << "\n";
    }
  }

  void printValueSet(llvm::raw_ostream &Stream,
                     const ValueSetT &ValueSet) const {
    Stream << "{";
    auto ValueIt = ValueSet.begin();
    if (ValueIt != ValueSet.end()) {
      Stream << (*ValueIt)->getName();
      ++ValueIt;
    }
    while (ValueIt != ValueSet.end()) {
      Stream << ", " << (*ValueIt)->getName();
      ++ValueIt;
    }
    Stream << "}";
  }

  void compute() {
    /* bool Changed = true; */
    /* while (Changed) { */
    /*   Changed = false; */
    /*   for (auto It = Blocks.rbegin(), End = Blocks.rend(); */
    /*        It != End; ++It) { */
    /*     updateLiveOut(*It); */
    /*     Changed = updateLiveIn(*It) || Changed; */
    /*   } */
    /* } */

    std::queue<const BasicBlock *> WorkList;
    for (auto It = Blocks.rbegin(), End = Blocks.rend();
         It != End; ++It)
      WorkList.push(*It);
    SmallPtrSet<const BasicBlock *, 8> QueuedBB(Blocks.begin(), Blocks.end());
    while (!WorkList.empty()) {
      const BasicBlock *BB = WorkList.front();
      WorkList.pop();
      QueuedBB.erase(BB);
      updateLiveOut(BB);
      if (updateLiveIn(BB)) {
        for (const BasicBlock *Pred : predecessors(BB)) {
          if (QueuedBB.count(Pred))
            continue;
          WorkList.push(Pred);
          QueuedBB.insert(Pred);
        }
      }
    }
  }

  bool updateLiveIn(const BasicBlock *Block) {
    auto &LiveIn = LiveIns[Block];
    const auto &LiveOut = LiveOuts[Block];
    const auto &Use = Uses[Block];
    const auto &Def = Defs[Block];
    size_t OldLiveInSize = LiveIn.size();
    for (const Value *V : LiveOut) {
      if (!Def.count(V))
        LiveIn.insert(V);
    }
    for (const Value *V : Use)
      LiveIn.insert(V);
    return LiveIn.size() > OldLiveInSize;
  }

  void updateLiveOut(const BasicBlock *Block) {
    auto &LiveOut = LiveOuts[Block];
    for (auto SuccIt = llvm::succ_begin(Block); SuccIt != llvm::succ_end(Block);
        ++SuccIt) {
      assert(LiveIns.count(*SuccIt) && "an unknown successive basic block");
      for (const Value *V : LiveIns[*SuccIt]) {
        LiveOut.insert(V);
      }
    }
  }

  const llvm::Function &Fun;
  std::vector<const BasicBlock *> Blocks;
  BlockValueMapT LiveIns;
  BlockValueMapT LiveOuts;
  BlockValueMapT Defs;
  BlockValueMapT Uses;
};

class LiveVariableAnalysis : public FunctionPass {
public:
  static char ID;

  explicit LiveVariableAnalysis() : FunctionPass(ID) {}

  bool runOnFunction(Function &F) override {
    Analyzer Ana(F);
    Ana.run();
    return false;
  }
};

char LiveVariableAnalysis::ID = 0;

RegisterPass<LiveVariableAnalysis> LiveVA(
    "live-variable", "Live variable analysis",
    true /* CFGOnly */, false /* is_analysis */);

} // end anonymous namespace
