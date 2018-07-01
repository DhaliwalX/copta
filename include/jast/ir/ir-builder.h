#ifndef IR_GENERATOR_H_
#define IR_GENERATOR_H_

#include "jast/ir/value.h"
#include "jast/ir/instruction.h"
#include "jast/ir/basic-block.h"

#include "jast/expression.h"
#include "jast/statement.h"

namespace jast {

class IRBuilder {
public:
  void SetFunction(Ref<Function> function) {
    currentFunction_ = function;
  }

  Ref<Function> GetFunction() {
    return currentFunction_;
  }

  void SetBasicBlock(Ref<BasicBlock> basicBlock) {
    currentBasicBlock_ = basicBlock;
  }

  Ref<BasicBlock> GetBasicBlock() {
    return currentBasicBlock_;
  }

  Ref<Value> getConstantInt(int64_t value) {
    return new ConstantInt(value);
  }

  Ref<Value> getConstantStr(const std::string &value) {
    return new ConstantStr(value);
  }

  template <OpCode op>
  Ref<Value> createBinary(Ref<Value> left, Ref<Value> right,
      const std::string &name = "") {
    return add(new BinaryInstruction<op>(left, right, name));
  }

  Ref<Value> createAlloc(Type *type, const std::string &name = "") {
    return add(new AllocInstruction(type, name));
  }

  Ref<Value> createStore(Ref<Value> value, Ref<Value> target,
    const std::string &name = "") {
    return add(new StoreInstruction(value, target, name));
  }

  Ref<Value> createLoad(Ref<Value> addr, const std::string &name = "") {
    return add(new LoadInstruction(addr, name));
  }

  Ref<Value> createBrk(Ref<Value> cond, Ref<BasicBlock> bb,
    const std::string &name = "") {
    return add(new BrkInstruction(cond, bb, name));
  }

  Ref<Value> createJmp(Ref<BasicBlock> to) {
    return add(new JmpInstruction(to));
  }

  Ref<Value> createGea(Ref<Value> base, Ref<ConstantStr> element,
    const std::string &name = "") {
    return add(new GeaInstruction(base, element, name));
  }

  Ref<Value> createIdx(Ref<Value> base, Ref<Value> idx,
    const std::string &name = "") {
    return add(new IdxInstruction(base, idx, name));
  }

  Ref<Value> createRet(Ref<Value> value) {
    return add(new RetInstruction(value, ""));
  }

  Ref<Value> createInvoke(Ref<Function> function, std::vector<Ref<Value>> args,
    const std::string &name = "") {
    return add(new InvokeInstruction(function, args, name));
  }

  Ref<Value> add(Ref<Value> value) {
    currentBasicBlock_->add(value);
    return value;
  }

private:
  Ref<Function> currentFunction_;
  Ref<BasicBlock> currentBasicBlock_;
};

}

#endif
