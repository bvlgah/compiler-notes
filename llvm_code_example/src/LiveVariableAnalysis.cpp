#include "llvm/ADT/PostOrderIterator.h"
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
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

using namespace llvm;

namespace {

class Analyzer {
public:
  explicit Analyzer(const Function &F)
      : Fun(F), Blocks(), LiveIns(), LiveOuts(), Kills(), Gens() {
    Blocks.reserve(Fun.size());
    ReversePostOrderTraversal<const Function *> RPO(&Fun);
    std::copy(RPO.begin(), RPO.end(), std::back_inserter(Blocks));
  }

  void run() {
    computeLiveInOut();
    print();
  }

private:
  using ValueSetT = std::unordered_set<const Value *>;
  using BlockValueMapT = std::unordered_map<const BasicBlock *, ValueSetT>;

  void print() const {
    llvm::errs() << "Result of live variable analyses for function '"
                 << Fun.getName() << "'\n";
    for (const BasicBlock *Block : Blocks) {
      llvm::errs() << "  live-in variables of block '"
                   << Block->getName() << "': ";
      auto LiveInIt = LiveIns.find(Block);
      assert(LiveInIt != LiveIns.end() && "an unknown basic block");
      printValueSet(llvm::errs(), LiveInIt->second);
      llvm::errs() << "\n";

      auto LiveOutIt = LiveOuts.find(Block);
      assert(LiveOutIt != LiveOuts.end() && "an unknown basic block");
      llvm::errs() << "  live-out variables of block '"
                   << Block->getName() << "': ";
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

  void computeLiveInOut() {
    for (const BasicBlock *Block : Blocks) {
      auto &Kill= Kills[Block];
      auto &Gen = Gens[Block];
      for (const Instruction &Inst : *Block) {
        for (const Use &U : Inst.operands()) {
          const Value *V = U.get();
          if (llvm::isa<llvm::Constant>(V)
              || V->getType()->getTypeID() == llvm::Type::LabelTyID)
            continue;
          if (!Kill.count(V))
            Gen.insert(V);
        }
        Kill.insert(&Inst);
      }
    }

    bool Changed = true;
    while (Changed) {
      Changed = false;
      for (auto BlockIt = Blocks.rbegin();
           BlockIt != Blocks.rend(); ++BlockIt) {
        Changed = updateLiveIn(*BlockIt) || Changed;
        Changed = updateLiveOut(*BlockIt) || Changed;
      }
    }
  }

  bool updateLiveIn(const BasicBlock *Block) {
    auto &LiveIn = LiveIns[Block];
    const auto &LiveOut = LiveOuts[Block];
    const auto &Gen = Gens[Block];
    const auto &Kill = Kills[Block];
    size_t OldLiveInSize = LiveIn.size();
    for (const Value *V : LiveOut) {
      if (!Kill.count(V))
        LiveIn.insert(V);
    }
    for (const Value *V : Gen)
      LiveIn.insert(V);
    return LiveIn.size() > OldLiveInSize;
  }

  bool updateLiveOut(const BasicBlock *Block) {
    auto &LiveOut = LiveOuts[Block];
    size_t OldLiveOutSize = LiveOut.size();
    for (auto SuccIt = llvm::succ_begin(Block); SuccIt != llvm::succ_end(Block);
        ++SuccIt) {
      assert(LiveIns.count(*SuccIt) && "an unknown successive basic block");
      for (const Value *V : LiveIns[*SuccIt]) {
        LiveOut.insert(V);
      }
    }
    return LiveOut.size() > OldLiveOutSize;
  }

  const llvm::Function &Fun;
  std::vector<const BasicBlock *> Blocks;
  BlockValueMapT LiveIns;
  BlockValueMapT LiveOuts;
  BlockValueMapT Kills;
  BlockValueMapT Gens;
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
