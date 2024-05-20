#pragma once

#include <llvm/IR/PassManager.h>
#include <llvm/Support/raw_ostream.h>

bool addLoopPrinter(llvm::FunctionPassManager &FPM, llvm::raw_ostream &OS);

void addLoopAnalysis(llvm::FunctionAnalysisManager &FAM);
