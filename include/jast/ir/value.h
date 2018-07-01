#ifndef VALUE_H_
#define VALUE_H_

#include "jast/ir/debug-info.h"

#include "jast/types/type.h"
#include "jast/list.h"
#include "jast/handle.h"

#include <iosfwd>
#include <list>

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

  void AddUser(Ref<Value> value) {
    use_list_.push_back(value);
  }

  virtual Type *getType() = 0;
private:
  std::list<Ref<Value>> use_list_;
  DebugInfo debug_info_;
};

class Void : public Value {
public:
  void print(std::ostream &os) const;
};

class Constant : public Value {
public:
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
    : value_{ val }
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
    : value_{val}
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
  explicit Parameter(Type *type)
    : type_{ type }
  { }

  Type *getType() override {
    return type_;
  }

  void print(std::ostream &os) const override;

private:
  Type *type_;
};

}

#endif
