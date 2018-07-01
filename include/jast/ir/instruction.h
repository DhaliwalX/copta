#ifndef INSTRUCTION_H_
#define INSTRUCTION_H_

#include "jast/types/type.h"
#include "jast/ir/value.h"
#include "jast/ir/function.h"
#include "jast/types/type-system.h"

namespace jast {

class BasicBlock;
class Function;

enum class OpCode {
#define R(I, _) k##I,
#include "instructions.h"
};

static inline std::string IrToString(OpCode op) {
  switch (op) {
#define R(I, S) case OpCode::k##I: return S;
#include "instructions.h"
  default:
    return "unknown";
  }
}

class Instruction : public Value {
public:
  Instruction(OpCode op, const std::string &name)
    : op_{ op }, name_{ name }
  { }

  virtual ~Instruction() = default;

  virtual void print(std::ostream &os) const;

  const OpCode &opCode() const { return op_; }

  std::vector<Ref<Value>> &operands() { return operands_; }

  bool IsInstruction() const override { return true; }
  Instruction *AsInstruction() override { return this; }

  Ref<Value> GetNthOperand(int i) {
    return operands_[i];
  }

  void SetName(std::string &name) {
    name_ = name;
  }
  void ShortPrint(std::ostream &os) const;
protected:
  void AddOperand(Ref<Value> value) {
    operands_.push_back(value);
    if (value)
      value->AddUser(this);
  }
  OpCode op_;
  std::vector<Ref<Value>> operands_;
protected:
  std::string name_;
};

template <OpCode op>
class BinaryInstruction : public Instruction {
public:
  BinaryInstruction(Ref<Value> first, Ref<Value> second, const std::string &name)
    : Instruction(op, name)
  {
    AddOperand(first);
    AddOperand(second);
  }

  Ref<Value> GetFirstOperand() {
    return GetNthOperand(0);
  }

  Ref<Value> GetSecondOperand() {
    return GetNthOperand(1);
  }

  Type *getType() override {
    return (GetNthOperand(0)->getType());
  }
};

#define B(I, _) using I##Instruction = BinaryInstruction<OpCode::k##I>;
#include "instructions.h"

class AllocInstruction : public Instruction {
public:
  AllocInstruction(Type *type, const std::string &name)
    : Instruction(OpCode::kAlloc, name), type_{type}
  { }

  Type *getType() override {
    return TypeSystem::getPointerType(type_);
  }

  void print(std::ostream &os) const override;

private:
  Type *type_;
};

class LoadInstruction : public Instruction {
public:
  LoadInstruction(Ref<Value> value, const std::string &name)
    : Instruction(OpCode::kLoad, name)
  {
    AddOperand(value);
  }

  Type *getType() override {
    assert(GetOperand()->getType()->IsPointerType());
    return GetOperand()->getType()->AsPointerType();
  }

  Ref<Value> GetOperand() {
    return GetNthOperand(0);
  }
};

class StoreInstruction : public Instruction {
public:
  StoreInstruction(Ref<Value> value, Ref<Value> target, const std::string &name)
    : Instruction(OpCode::kStore, name)
  {
    AddOperand(value);
    AddOperand(target);
  }

  Ref<Value> GetSource() {
    return GetNthOperand(0);
  }

  Ref<Value> GetTarget() {
    return GetNthOperand(1);
  }

  Type *getType() override {
    return TypeSystem::getUndefinedType();
  }
};

class BrkInstruction : public Instruction {
public:
  BrkInstruction(Ref<Value> cond, Ref<BasicBlock> bb, const std::string &name)
    : Instruction(OpCode::kBrk, name)
  {
    AddOperand(cond);
    AddOperand(bb);
  }

  Ref<Value> GetCondition() {
    return GetNthOperand(0);
  }

  Ref<BasicBlock> GetBlock() {
    return GetNthOperand(1)->AsBasicBlock();
  }

  Type *getType() override {
    return TypeSystem::getUndefinedType();
  }
};

class JmpInstruction : public Instruction {
public:
  JmpInstruction(Ref<BasicBlock> bb, const std::string &name = "")
    : Instruction(OpCode::kJmp, name)
  {
    AddOperand(bb);
  }

  Ref<BasicBlock> GetBlock() {
    return GetNthOperand(0)->AsBasicBlock();
  }

  Type *getType() override {
    return TypeSystem::getUndefinedType();
  }
};

class AddrOfInstruction : public Instruction {
public:
  AddrOfInstruction(Ref<Value> base, const std::string &name)
    : Instruction(OpCode::kAddrOf, name)
  {
    AddOperand(base);
  }

  Ref<Value> GetBase() {
    return GetNthOperand(0);
  }

  Type *getType() override {
    return TypeSystem::getPointerType(GetBase()->getType());
  }
};

class GeaInstruction : public Instruction {
public:
  GeaInstruction(Ref<Value> base, Ref<ConstantStr> element, const std::string &name)
    : Instruction(OpCode::kGea, name)
  {
    AddOperand(base);
    AddOperand(element);
  }

  Ref<Value> GetBase() {
    return GetNthOperand(0);
  }

  std::string GetElement() {
    return GetNthOperand(1)->AsConstant()->AsStr();
  }

  Type *getType() override {
    assert(GetBase()->getType()->IsObjectType());
    return GetBase()->getType()->AsObjectType()->getMember(GetElement());
  }
};

class IdxInstruction : public Instruction {
public:
  IdxInstruction(Ref<Value> base, Ref<Value> idx, const std::string &name)
    : Instruction(OpCode::kIdx, name)
  {
    AddOperand(base);
    AddOperand(idx);
  }

  Ref<Value> GetBase() {
    return GetNthOperand(0);
  }

  Ref<Value> GetIndex() {
    return GetNthOperand(1);
  }

  Type *getType() override {
    assert(GetBase()->getType()->IsArrayType());
    return GetBase()->getType()->AsArrayType()->getBaseElementType();
  }
};

class RetInstruction : public Instruction {
public:
  RetInstruction(Ref<Value> value, const std::string &name)
    : Instruction(OpCode::kRet, name)
  {
    AddOperand(value);
  }

  Ref<Value> GetValue() {
    return GetNthOperand(0);
  }

  Type *getType() override {
    return TypeSystem::getUndefinedType();
  }
};

class InvokeInstruction : public Instruction {
public:
  InvokeInstruction(Ref<Function> function, std::vector<Ref<Value>> args,
      const std::string &name)
    : Instruction(OpCode::kInvoke, name), function_{function}
  {
    for (auto &arg : args) {
      AddOperand(arg);
    }
  }

  Ref<Function> GetFunction() {
    return function_;
  }

  Ref<Value> GetNthArg(int i) {
    return GetNthOperand(i);
  }

  Type *getType() override {
    return function_->getReturnType();
  }

  void print(std::ostream &os) const override;

private:
  Ref<Function> function_;
};

}

#endif
