#include "SCCP.h"

#include <llvm/Analysis/SparsePropagation.h>
#include <llvm/IR/Argument.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/ErrorHandling.h>

#include <cassert>
#include <utility>

using namespace llvm;

namespace {

class MySCCPPass : public llvm::PassInfoMixin<MySCCPPass> {
public:
  llvm::PreservedAnalyses run(llvm::Function &F,
                              llvm::FunctionAnalysisManager &AM);
};

using SCCPLatticeKey = Value *;

class SCCPLatticeValue {
public:
  enum LatticeType {
    Undefined,
    Const,
    Overdefined,
    Untracked,
  };

  SCCPLatticeValue(const SCCPLatticeValue &Other) = default;
  SCCPLatticeValue &operator=(const SCCPLatticeValue &Other) = default;

#define SPECIAL_TYPE_VISIT(V)                                             \
  V(Undefined)                                                            \
  V(Overdefined)                                                          \
  V(Untracked)

#define TYPE_VISIT(V)                                                     \
  V(Undefined)                                                            \
  V(Const)                                                                \
  V(Overdefined)                                                          \
  V(Untracked)

#define DEFINE_SPECIAL_GETTER(LatticeTy)                                  \
  static SCCPLatticeValue get##LatticeTy() {                              \
    return SCCPLatticeValue(LatticeTy, nullptr);                          \
  }

  SPECIAL_TYPE_VISIT(DEFINE_SPECIAL_GETTER)

  LatticeType getType() const { return Type; }

  Constant *getConstantVal() const { return C; }

#define DEFINE_TYPE_CHECKER(LatticeTy)                                    \
  bool is##LatticeTy() const {                                            \
    return getType() == LatticeTy;                                        \
  }

  TYPE_VISIT(DEFINE_TYPE_CHECKER)

  SCCPLatticeValue merge(const SCCPLatticeValue &Other) const {
    if (isOverdefined() || Other.isOverdefined())
      return getOverdefined();
    if (isUntracked() || Other.isUntracked())
      return getUntracked();

    if (isUndefined())
      return Other;
    else if(Other.isUndefined())
      return *this;

    assert(isConst() && Other.isConst() &&
           "expect two constant lattice values");
    if (C == Other.C)
      return *this;

    return getOverdefined();
  }

  static SCCPLatticeValue createConst(Constant *C) {
    return SCCPLatticeValue(Const, C);
  }

  bool operator==(const SCCPLatticeValue &Other) const {
    if (getType() != Other.getType())
      return false;
    if (getType() == Const && getConstantVal() != Other.getConstantVal())
      return false;
    return true;
  }

  SCCPLatticeValue() : Type(Undefined), C(nullptr) {}

private:
  explicit SCCPLatticeValue(LatticeType Type, Constant *C)
      : Type(Type), C(C) {}
  LatticeType Type;
  Constant *C;
};

class SCCPLatticeFunction : public AbstractLatticeFunction<SCCPLatticeKey,
                                                           SCCPLatticeValue> {
public:
  SCCPLatticeValue ComputeLatticeVal(SCCPLatticeKey Key) override {
    Value *V = Key;
    if (isa<Argument>(V) || isa<LoadInst>(V))
      return getOverdefinedVal();
    return getUndefVal();
  }

  SCCPLatticeValue MergeValues(SCCPLatticeValue X,
                               SCCPLatticeValue Y) override {
    return X.merge(Y);
  }

  void ComputeInstructionState(Instruction &I,
      DenseMap<SCCPLatticeKey, SCCPLatticeValue> &ChangedValues,
      SparseSolver<SCCPLatticeKey, SCCPLatticeValue> &Solver) override {
    if (isa<UnaryOperator>(I))
      computeUnary(dyn_cast<UnaryOperator>(&I), ChangedValues, Solver);
    else if (isa<BinaryOperator>(I))
      computeBinary(dyn_cast<BinaryOperator>(&I), ChangedValues, Solver);
  }

  Value *GetValueFromLatticeVal(SCCPLatticeValue Val, Type *Ty) override {
    if (!Val.isConst())
      return nullptr;

    Constant *C = Val.getConstantVal();
    assert(C && "lattice value representing a constant is nullptr");
    if (Ty && C->getType() != Ty)
      return nullptr;
    else
      return C;
  }

private:
  void computeUnary(UnaryOperator *Unary,
      DenseMap<SCCPLatticeKey, SCCPLatticeValue> &ChangedValues,
      SparseSolver<SCCPLatticeKey, SCCPLatticeValue> &Solver) {
    if (!Unary || Unary->getOpcode() != Instruction::FNeg)
      return;

    SCCPLatticeValue Val = Solver.getValueState(Unary->getOperand(0));
    switch(Val.getType()) {
      case SCCPLatticeValue::Undefined:
        ChangedValues.insert(
            std::make_pair(Unary, SCCPLatticeValue::getUndefined()));
        break;
      case SCCPLatticeValue::Const:
        if (ConstantFP *FP = dyn_cast<ConstantFP>(Val.getConstantVal())) {
          ConstantFP *Neg = ConstantFP::get(Unary->getContext(),
                                            neg(FP->getValue()));
          ChangedValues.insert(
              std::make_pair(Unary, SCCPLatticeValue::createConst(Neg)));
        }
      case SCCPLatticeValue::Overdefined:
        ChangedValues.insert(
            std::make_pair(Unary, SCCPLatticeValue::getOverdefined()));
        break;
      case SCCPLatticeValue::Untracked:
        ChangedValues.insert(
            std::make_pair(Unary, SCCPLatticeValue::getUntracked()));
        break;
      default:
        llvm_unreachable("unknown lattice type");
    }
  }

  void computeBinary(BinaryOperator *Binary,
      DenseMap<SCCPLatticeKey, SCCPLatticeValue> &ChangedValues,
      SparseSolver<SCCPLatticeKey, SCCPLatticeValue> &Solver) {
  }
};

PreservedAnalyses MySCCPPass::run(Function &F, FunctionAnalysisManager &AM) {
  return PreservedAnalyses::none();
}

} // end anonymous namespace

namespace llvm {

template <>
class LatticeKeyInfo<SCCPLatticeKey> {
public:
  static inline Value *getValueFromLatticeKey(SCCPLatticeKey Key) {
    return Key;
  }

  static inline SCCPLatticeKey getLatticeKeyFromValue(Value *V) { return V; }
};

} // end namespace llvm

bool addSCCP(llvm::FunctionPassManager &FPM) {
  FPM.addPass(MySCCPPass());
  return true;
}
