#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
struct Hello : public FunctionPass {
  static char ID;
  Hello() : FunctionPass(ID) {}

  bool runOnFunction(Function &F) override {
    errs() << "Hello: function '";
    errs().write_escaped(F.getName()) << "'\n";
    for (const llvm::BasicBlock &BB : F) {
      errs() << "  basic block : '";
      errs().write_escaped(BB.getName()) << "'\n";
      for (const llvm::Instruction &Inst : BB) {
        errs() << "    " << Inst.getOpcodeName() << '\n';
      }
    }
    return false;
  }
}; // end of struct Hello
}  // end of anonymous namespace

char Hello::ID = 0;
static RegisterPass<Hello> X("myhello", "Hello World Pass",
                             false /* Only looks at CFG */,
                             false /* Analysis Pass */);
