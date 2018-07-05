#ifndef FUNCTION_H_
#define FUNCTION_H_

#include "jast/ir/value.h"

namespace jast {

class BasicBlock;

class Function : public Value {
public:
  using iterator = std::vector<Ref<BasicBlock>>::iterator;

  Function(std::vector<Ref<BasicBlock>> bbs, FunctionType *t, const std::string &name)
    : bbs_{bbs}, type_{t}, name_{ name }
  { }

  Function() = default;

  void print(std::ostream &os) const override;
  void dump() const;

  Function *AsFunction() override {
    return this;
  }

  void setExtern(bool e) { extern_ = e; }
  bool isExtern() const { return extern_; }

  Ref<BasicBlock> getEntry() {
    return bbs_[0];
  }

  iterator begin() {
    return bbs_.begin();
  }

  iterator end() {
    return bbs_.end();
  }

  Type *getType() override {
    return type_;
  }

  void setType(Type *type) {
    type_ = type->AsFunctionType();
  }

  void setName(std::string name) {
    name_ = name;
  }

  void AddBB(Ref<BasicBlock> bb) {
    bbs_.push_back(bb);
  }

  const std::string &getName() const {
    return name_;
  }

  Type *getReturnType() {
    return getType()->AsFunctionType()->getReturnType();
  }

private:
  std::vector<Ref<BasicBlock>> bbs_;
  FunctionType *type_;
  std::string name_;
  bool extern_;
};

}

#endif
