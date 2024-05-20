#include "LiveVariableAnalysis.h"
#include "Loop.h"
#include "SCCP.h"

#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/Compiler.h"

using namespace llvm;

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "MyPasses", "v0.1",
          [](PassBuilder &Builder) {
            Builder.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "live-variable")
                    return addLiveVariableAnalysis(FPM);
                  else if (Name == "mysccp")
                    return addSCCP(FPM);
                  else if (Name == "my-loop-printer")
                    return addLoopPrinter(FPM, llvm::errs());
                  else
                    return false;
                });
            Builder.registerAnalysisRegistrationCallback(
                [](FunctionAnalysisManager &FAM) {
                  addLoopAnalysis(FAM);
                });
          }};
}
