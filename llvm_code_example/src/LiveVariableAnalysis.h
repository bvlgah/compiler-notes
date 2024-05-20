#pragma once

#include <llvm/IR/PassManager.h>

bool addLiveVariableAnalysis(llvm::FunctionPassManager &FPM);
