#pragma once

#include <llvm/IR/PassManager.h>

bool addSCCP(llvm::FunctionPassManager &FPM);
