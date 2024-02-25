#include "llvm/ADT/DepthFirstIterator.h"
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
#include <deque>
#include <iterator>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

using namespace llvm;

namespace {

class Analyzer {
public:
  explicit Analyzer()
      : Fun(nullptr), LiveIns(), LiveOuts(), Defs(), Uses() {}

  void run(const Function &F) {
    init(F);
    compute();
  }

  void clear() {
    Fun = nullptr;
    LiveIns.clear();
    LiveOuts.clear();
    Defs.clear();
    Uses.clear();
  }

  void print(raw_ostream &OS) const {
    assert(Fun && "Function not specified");
    OS << "Result of live variable analyses for function '"
       << Fun->getName() << '\'' << '\n';
    for (const BasicBlock &Block : *Fun) {
      OS << "  live-in variables of block '"
         << Block.getName() << "': ";
      auto LiveInIt = LiveIns.find(&Block);
      assert(LiveInIt != LiveIns.end() && "an unknown basic block");
      printValueSet(llvm::errs(), LiveInIt->second);
      OS << '\n';

      auto LiveOutIt = LiveOuts.find(&Block);
      assert(LiveOutIt != LiveOuts.end() && "an unknown basic block");
      OS << "  live-out variables of block '"
         << Block.getName() << "': ";
      printValueSet(llvm::errs(), LiveOutIt->second);
      OS << '\n';
    }
  }

private:
  using ValueSetT = std::unordered_set<const Value *>;
  using BlockValueMapT = std::unordered_map<const BasicBlock *, ValueSetT>;

  void init(const Function &F) {
    Fun = &F;
    for (const BasicBlock &Block : *Fun) {
      LiveIns.emplace(&Block, ValueSetT());
      LiveOuts.emplace(&Block, ValueSetT());
      BlockValueMapT::iterator DefIt;
      BlockValueMapT::iterator UseIt;
      std::tie(DefIt, std::ignore) = Defs.emplace(&Block, ValueSetT());
      std::tie(UseIt, std::ignore) = Uses.emplace(&Block, ValueSetT());

      for (const Instruction &Inst : Block) {
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
    // Since live variable analysis is a backward problem, it is efficient to
    // iterate in reverse preorder (live variables for successors of a node are
    // computed first). And unreachable basic blocks will not be iterated over.
    std::deque<const BasicBlock *> WorkList(df_begin(Fun), df_end(Fun));
    errs() << "WorkList.size() = " << WorkList.size()
           << ", Fun.size() = " << Fun->size() << '\n';

    SmallPtrSet<const BasicBlock *, 8> QueuedBB(WorkList.begin(),
                                                WorkList.end());
    while (!WorkList.empty()) {
      const BasicBlock *BB = WorkList.front();
      WorkList.pop_front();
      QueuedBB.erase(BB);
      updateLiveOut(BB);
      if (updateLiveIn(BB)) {
        for (const BasicBlock *Pred : predecessors(BB)) {
          if (QueuedBB.count(Pred))
            continue;
          WorkList.push_back(Pred);
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

  const Function *Fun;
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
    LA.run(F);
    return false;
  }

  void print(raw_ostream &OS, const Module *) const override {
    LA.print(OS);
  }

  void releaseMemory() override {
    LA.clear();
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  }

private:
  Analyzer LA;
};

char LiveVariableAnalysis::ID = 0;

RegisterPass<LiveVariableAnalysis> LiveVA(
    "live-variable", "Live variable analysis",
    true /* CFGOnly */, false /* is_analysis */);

} // end anonymous namespace
