#ifndef VALUE_H_
#define VALUE_H_

#include "jast/ir/debug-info.h"

#include "jast/types/type.h"
#include "jast/types/type-system.h"
#include "jast/list.h"
#include "jast/handle.h"

#include <iosfwd>
#include <list>
#include <algorithm>

namespace jast {

class BasicBlock;
class Function;
class Instruction;
class Constant;

class Value : public RefCountObject {
public:
  Value() = default;
  virtual ~Value() {}
  virtual void print(std::ostream &os) const = 0;

  DebugInfo &debug_info() { return debug_info_; }

  auto &user_list() { return user_list_; }
  const auto &user_list() const { return user_list_; }

  auto &use_list() { return use_list_; }
  const auto &use_list() const { return use_list_; }

  virtual bool IsConstant() const { return false; }
  virtual bool IsInstruction() const { return false; }
  virtual bool IsBasicBlock() const { return false; }

  virtual Constant *AsConstant() { return nullptr; }
  virtual Instruction *AsInstruction() { return nullptr; }
  virtual BasicBlock *AsBasicBlock() { return nullptr; }
  virtual Function *AsFunction() { return nullptr; }

  void SetDebugInfo(const DebugInfo &debug_info) {
    debug_info_ = debug_info;
  }

  void RemoveUse(Ref<Value> use) {
    std::remove_if(use_list_.begin(), use_list_.end(), [&](Ref<Value> &u) { return u == use; });
  }

  void RemoveUser(Ref<Value> user) {
    std::remove_if(user_list_.begin(), user_list_.end(), [&](Ref<Value> &u) { return u == user; });
  }

  void AddUser(Ref<Value> value) {
    user_list_.push_back(value);
    value->AddUse(this);
  }

  void AddUse(Ref<Value> value) {
    use_list_.push_back(value);
  }

  virtual Type *getType() = 0;
private:
  std::list<Ref<Value>> use_list_;
  std::list<Ref<Value>> user_list_;
  DebugInfo debug_info_;
};

class Void : public Value {
public:
  void print(std::ostream &os) const;
};

class Constant : public Value {
public:
  Constant(Type *type)
    : type_{ type }
  { }
  virtual int64_t AsInt() const = 0;
  virtual std::string AsStr() const = 0;
  virtual bool IsInt() const { return type_->IsIntegerType(); }
  virtual bool IsStr() const { return type_->IsStringType(); }

  Constant *AsConstant() { return this; }
  Type *getType() override { return type_; }
  bool IsConstant() const override { return true; }
private:
  Type *type_;
};

class ConstantInt : public Constant {
public:
  explicit ConstantInt(int64_t val)
    : Constant(TypeSystem::getIntegerType()), value_{ val }
  { }
  int64_t AsInt() const override {
    return value_;
  }

  std::string AsStr() const override {
    return std::to_string(value_);
  }

  void print(std::ostream &os) const override;
private:
  int64_t value_;
};

class ConstantStr : public Constant {
public:
  explicit ConstantStr(const std::string &val)
    : Constant(TypeSystem::getStringType()), value_{val}
  { }

  std::string AsStr() const override {
    return value_;
  }

  int64_t AsInt() const override {
    return 0;
  }

  void print(std::ostream &os) const override;

private:
  std::string value_;
};

// special class for holding the logic for the parameter
class Parameter : public Value {
public:
  explicit Parameter(Type *type, int id)
    : type_{ type }, id_{id}
  { }

  Type *getType() override {
    return type_;
  }

  int getId() const {
    return id_;
  }

  void print(std::ostream &os) const override;

private:
  Type *type_;
  int id_;
};

}

#endif
