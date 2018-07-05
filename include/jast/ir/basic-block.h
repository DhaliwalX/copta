#ifndef BASIC_BLOCK_H_
#define BASIC_BLOCK_H_

#include "jast/ir/value.h"

namespace jast {

class BasicBlock : public Value {
public:
  using iterator = std::list<Ref<Instruction>>::iterator;

  BasicBlock(Ref<Function> function, const std::string &name = "")
    : function_{function}, label_{ name }
  { }

  std::list<Ref<Instruction>> instructions() { return instructions_; }

  std::list<Ref<Instruction>>::iterator begin() { return instructions_.begin(); }
  std::list<Ref<Instruction>>::iterator end() { return instructions_.end(); }

  void remove(std::list<Ref<Instruction>>::iterator i) {
    instructions_.erase(i);
  }

  bool IsBasicBlock() const override {
    return true;
  }
  BasicBlock *AsBasicBlock() override {
    return this;
  }

  iterator insert(iterator pos, Ref<Instruction> i) {
    return instructions_.insert(pos, i);
  }

  void setLabel(std::string label) {
    label_ = label;
  }

  const std::string &label() const { return label_; }

  void add(Ref<Value> i) {
    instructions_.push_back(i);
  }
  void print(std::ostream &os) const override;

  Type *getType() override {
    return nullptr;
  }
  void dump() const;
private:
  Ref<Function> function_;
  std::list<Ref<Instruction>> instructions_;
  std::string label_;
};

}

#endif
