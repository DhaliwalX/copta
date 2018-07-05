#include "jast/ir/interp.h"
#include "jast/types/type-system.h"
#include "jast/ir/module.h"
#include "jast/ir/function.h"
#include "jast/ir/instruction.h"
#include "jast/ir/basic-block.h"
#include "jast/expression.h"
#include "jast/statement.h"

#include <iostream>

namespace jast {
namespace internal {

std::map<std::string, NativeFunction> NativeFunctionResolver::functions_;

#define FAIL(message) assert(false && message && __LINE__ && ":" && __FILE__)

#define FAIL_IF(cond, message) assert(!(cond) && message && __LINE__ && ":" && __FILE__)
class InterpreterImpl {
public:
  bool Execute(Ref<Module> m) {
    auto init = m->get("init");
    auto firstblock = init->getEntry();
    current_ = firstblock;
    next_ = current_->begin();
    while (true) {
      auto i = next_++;
      std::cout << "-- ";
      (*i)->print(std::cout);
      std::cout << std::endl;
      Execute(*i);

      if (current_->end() == next_) {
        break;
      }
    }

    return true;
  }

  Ref<Object> NewObject(Type *t) {
    switch (t->getTypeID()) {
      case TypeID::Integer:
        return Ref<Object>(new Object(kNumber));

      case TypeID::String:
        return Ref<Object>(new Object(kString));
      case TypeID::Array:
        return Ref<Object>(new Object(kArray));
      case TypeID::Object:
        return Ref<Object>(new Object(kStruct));
      case TypeID::Pointer:
        return Ref<Object>(new Object(kPointer));
      default:
        return Ref<Object>(new Object(kUndefined));
    }
  }

  Ref<Object> NewObject(ObjectType t) {
    return Ref<Object>(new Object(t));
  }

  Ref<Object> Execute(Ref<AllocInstruction> a) {
    valueMap_[a.get()] = NewObject(a->getType());
    return nullptr;
  }

  Ref<Object> Execute(Ref<LoadInstruction> l) {
    auto obj = valueMap_[l->GetOperand().get()];
    assert(obj->Type == kPointer && "obj type is not pointer");
    valueMap_[l.get()] = obj->Pointer;
    return obj->Pointer;
  }

  Ref<Object> Execute(Ref<StoreInstruction> s) {
    auto location = valueMap_[s->GetSource().get()];
    decltype(location) target;
    if (s->GetTarget()->IsConstant()) {
      auto c = cast<Constant, Value>(s->GetTarget());
      target = NewObject(kNumber);
      if (c->IsInt()) {
        target->Int = c->AsInt();
      } else {
        target->Type = kString;
        target->Str = c->AsStr();
      }
    } else {
      target = valueMap_[s->GetTarget().get()];
    }
    assert(location->Type == kPointer && "location is not a pointer");
    location->Pointer = target;
    return nullptr;
  }

  Ref<Object> FromConstant(Ref<Constant> c) {
    auto target = NewObject(kNumber);
    if (c->IsInt()) {
      target->Int = c->AsInt();
    } else {
      target->Type = kString;
      target->Str = c->AsStr();
    }

    return target;
  }

  Ref<Object> ConstantOrValue(Ref<Value> v) {
    if (v->IsConstant()) {
      return FromConstant(cast<Constant, Value>(v));
    } else {
      return valueMap_[v.get()];
    }
  }

  template <OpCode op>
  Ref<Object> Execute(Ref<BinaryInstruction<op>> b) {
    Ref<Object> lhs, rhs;

    lhs = ConstantOrValue(b->GetFirstOperand());
    rhs = ConstantOrValue(b->GetSecondOperand());

    if (op == OpCode::kAdd) {
      if (lhs->Type == kString &&rhs->Type == kString) {
        auto result = NewObject(kString);
        result->Str = lhs->Str + rhs->Str;
        valueMap_[b.get()] = result;
        return result;
      } else if (lhs->Type == kNumber && rhs->Type == kNumber) {
        auto result = NewObject(kNumber);
        result->Int = lhs->Int + rhs->Int;
        valueMap_[b.get()] = result;
        return result;
      } else {
        FAIL("Unable to perform addition operation between incompatible types");
      }
    }

    if (lhs->Type != kNumber && rhs->Type != kNumber) {
      FAIL("Unable to perform binary operation between incompatible types");
    }

    auto result = NewObject(kNumber);
    auto l = lhs->Int;
    auto r = rhs->Int;
    decltype(l) num = 0;
    switch (op) {
      case OpCode::kSub:
        num = l - r;
        break;
      case OpCode::kMul:
        num = l *r;
        break;
      case OpCode::kDiv:
        num = l / r;
        break;
      case OpCode::kMod:
        num = l % r;
        break;
      case OpCode::kShr:
        num = l >> r;
        break;
      case OpCode::kShl:
        num = l << r;
        break;
      case OpCode::kSzr:
        num = l >> r;
        break;
      case OpCode::kLt:
        num = l < r;
        break;
      case OpCode::kGt:
        num = l > r;
        break;
      case OpCode::kEq:
        num = l== r;
        break;
      case OpCode::kNeq:
        num = l != r;
        break;
      case OpCode::kAnd:
        num = l && r;
        break;
      case OpCode::kOr:
        num = l || r;
        break;
      case OpCode::kXor:
        num = l ^ r;
        break;
      default:
      FAIL("Unrecognized binary operation");
    }

    result->Int = num;
    valueMap_[b.get()] = result;
    return result;
  }

  void PerformJump(Ref<BasicBlock> bb) {
    current_ = bb;
    next_ = current_->begin();
  }

  Ref<Object> Execute(Ref<BrkInstruction> b) {
    auto cond = ConstantOrValue(b->GetCondition());
    FAIL_IF(cond->Type != kNumber, "condition should be an integer");
    if (cond->Int) {
      PerformJump(b->GetBlock());
    }
    return nullptr;
  }

  Ref<Object> Execute(Ref<JmpInstruction> j) {
    PerformJump(j->GetBlock());
    return nullptr;
  }

  Ref<Object> Execute(Ref<GeaInstruction> g) {
    auto obj = valueMap_[g->GetBase().get()];
    FAIL_IF(obj->Type != kStruct, "type should be object for gea instruction");
    auto result = NewObject(kPointer);
    valueMap_[g.get()] = result;
    result->Pointer = obj->Struct[g->GetElement()];
    return result;
  }

  Ref<Object> Execute(Ref<IdxInstruction> i) {
    auto obj = valueMap_[i->GetBase().get()];
    FAIL_IF(obj->Type != kArray, "type should be array for idx instruction");
    auto result = NewObject(kPointer);
    valueMap_[i.get()] = result;
    auto idx = valueMap_[i->GetIndex().get()];
    FAIL_IF(idx->Type != kNumber, "type of index should be an integer");
    result->Pointer = obj->Arr[idx->Int];
    return result;
  }

  Ref<Object> Execute(Ref<RetInstruction> r) {
    auto obj = ConstantOrValue(r->GetValue());
    isReturn_ = true;
    retResult_ = obj;
    return obj;
  }

  Ref<Object> Execute(Ref<InvokeInstruction> i) {
    auto f = i->GetFunction();

    // save the iterators
    auto next_S = next_;
    auto current_S = current_;
    auto isReturn_S = isReturn_;

    std::vector<Ref<Object>> args;

    for (auto &arg : i->operands()) {
      auto obj = ConstantOrValue(arg);
      args.push_back(obj);
    }

    if (f->isExtern()) {
      // execute native function
      auto nf = NativeFunctionResolver::Resolve(f->getName());
      auto r = nf(args);
      valueMap_[i.get()] = r;
      return r;
    }

    isReturn_ = false;
    current_ = f->getEntry();
    next_ = current_->begin();
    while (true) {
      auto i = next_++;
      Execute(*i);

      if (current_->end() == next_) {
        break;
      }
    }

    Ref<Object> result = nullptr;
    if (isReturn_)
      result = retResult_;

    next_ = next_S;
    current_ = current_S;
    isReturn_ = isReturn_S;
    valueMap_[i.get()] = result;
    return result;
  }

  Ref<Object> Execute(Ref<Instruction> i) {
    switch (i->opCode()) {
      case OpCode::kAlloc:
        Execute(cast<AllocInstruction, Instruction>(i));
        break;
      case OpCode::kLoad:
        Execute(cast<LoadInstruction, Instruction>(i));
        break;
      case OpCode::kStore:
        Execute(cast<StoreInstruction, Instruction>(i));
        break;
      case OpCode::kAdd:
        Execute(cast<BinaryInstruction<OpCode::kAdd>, Instruction>(i));
        break;
      case OpCode::kSub:
        Execute(cast<BinaryInstruction<OpCode::kSub>, Instruction>(i));
        break;
      case OpCode::kMul:
        Execute(cast<BinaryInstruction<OpCode::kMul>, Instruction>(i));
        break;
      case OpCode::kDiv:
        Execute(cast<BinaryInstruction<OpCode::kDiv>, Instruction>(i));
        break;
      case OpCode::kMod:
        Execute(cast<BinaryInstruction<OpCode::kMod>, Instruction>(i));
        break;
      case OpCode::kShr:
        Execute(cast<BinaryInstruction<OpCode::kShr>, Instruction>(i));
        break;
      case OpCode::kShl:
        Execute(cast<BinaryInstruction<OpCode::kShl>, Instruction>(i));
        break;
      case OpCode::kSzr:
        Execute(cast<BinaryInstruction<OpCode::kSzr>, Instruction>(i));
        break;
      case OpCode::kLt:
        Execute(cast<BinaryInstruction<OpCode::kLt>, Instruction>(i));
        break;
      case OpCode::kGt:
        Execute(cast<BinaryInstruction<OpCode::kGt>, Instruction>(i));
        break;
      case OpCode::kEq:
        Execute(cast<BinaryInstruction<OpCode::kEq>, Instruction>(i));
        break;
      case OpCode::kNeq:
        Execute(cast<BinaryInstruction<OpCode::kNeq>, Instruction>(i));
        break;
      case OpCode::kAnd:
        Execute(cast<BinaryInstruction<OpCode::kAnd>, Instruction>(i));
        break;
      case OpCode::kOr:
        Execute(cast<BinaryInstruction<OpCode::kOr>, Instruction>(i));
        break;
      case OpCode::kXor:
        Execute(cast<BinaryInstruction<OpCode::kXor>, Instruction>(i));
        break;
      case OpCode::kBrk:
        Execute(cast<BrkInstruction, Instruction>(i));
        break;
      case OpCode::kJmp:
        Execute(cast<JmpInstruction, Instruction>(i));
        break;
      case OpCode::kGea:
        Execute(cast<GeaInstruction, Instruction>(i));
        break;
      case OpCode::kIdx:
        Execute(cast<IdxInstruction, Instruction>(i));
        break;
      case OpCode::kRet:
        Execute(cast<RetInstruction, Instruction>(i));
        break;
      case OpCode::kInvoke:
        Execute(cast<InvokeInstruction, Instruction>(i));
        break;
      default:
        assert(false && "Not implemented interp function");
    }
    return nullptr;
  }

private:
  std::map<Value*, Ref<Object>> valueMap_;
  Ref<BasicBlock> current_;
  BasicBlock::iterator next_;
  Ref<Object> retResult_;
  bool isReturn_;
};

bool Interpreter::Execute(Ref<Module> m) {
  InterpreterImpl impl;
  return impl.Execute(m);
}

}
}
