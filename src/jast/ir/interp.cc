#include "jast/ir/interp.h"
#include "jast/types/type-system.h"
#include "jast/ir/module.h"
#include "jast/ir/function.h"
#include "jast/ir/instruction.h"
#include "jast/ir/basic-block.h"
#include "jast/expression.h"
#include "jast/types/type.h"
#include "jast/statement.h"

#include <iostream>

namespace jast {
namespace internal {

std::map<std::string, NativeFunction> NativeFunctionResolver::functions_;

#define FAIL(message) assert(false && message && __LINE__ && ":" && __FILE__)

#define FAIL_IF(cond, message) assert(!(cond) && message && __LINE__ && ":" && __FILE__)
class InterpreterImpl {
public:

  bool Tick() {
    if (current_->end() == next_) {
      return false;
    }
    auto i = next_++;

    // std::cout << ticks_++ << ": ";
    // (*i)->print(std::cout);
    // std::cout << std::endl;

    Execute(*i);
    return true;
  }

  void Loop() {
    bool should_continue = true;
    while (should_continue) {
      should_continue = Tick();
    }
  }
  bool Execute(Ref<Module> m) {
    ticks_ = 0;
    auto init = m->get("init");
    auto firstblock = init->getEntry();
    current_ = firstblock;
    next_ = current_->begin();
    Loop();
    return true;
  }

  Ref<Object> NewStruct(jast::ObjectType *t) {
    auto obj = Ref<Object>(new Object(kStruct));
    for (auto &prop : *t) {
      obj->Struct[prop.first] = NewObject(prop.second);
    }

    return obj;
  }

  Ref<Object> NewObject(Type *t) {
    if (t->IsPointerType()) {
      t = t->AsPointerType()->getBaseElementType();
    }
    Ref<Object> result;
    switch (t->getTypeID()) {
      case TypeID::Integer:
        result = Ref<Object>(new Object(kNumber));
        break;

      case TypeID::String:
        result = Ref<Object>(new Object(kString));
        break;
      case TypeID::Array:
        result = Ref<Object>(new Object(kArray));
        break;
      case TypeID::Object:
        result = NewStruct(t->AsObjectType());
        break;
      case TypeID::Pointer:
        result = Ref<Object>(new Object(kPointer));
        break;
      default:
        result = Ref<Object>(new Object(kUndefined));
    }

    auto p = NewObject(kPointer);
    p->Pointer = result;
    return p;
  }

  Ref<Object> NewObject(ObjectType t) {
    return Ref<Object>(new Object(t));
  }

  Ref<Object> Execute(Ref<AllocInstruction> a) {
    valueMap_[a.get()] = NewObject(a->getType());
    return nullptr;
  }

  Ref<Object> Execute(Ref<LoadInstruction> l) {
    if (auto param = cast<Parameter, Value>(l->GetOperand())) {
      valueMap_[l.get()] = args_[param->getId()];
      return args_[param->getId()];
    }
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
    CopyTo(location->Pointer, target);
    return nullptr;
  }

  void CopyTo(Ref<Object> pointer, Ref<Object> from) {
    assert(pointer && "null pointer");
    pointer->Int = from->Int;
    pointer->Str = from->Str;
    pointer->Struct = from->Struct;
    pointer->Arr = from->Arr;
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
    FAIL_IF(obj->Type != kPointer, "type should be pointer to object for gea instruction");
    obj = obj->Pointer;
    FAIL_IF(obj->Type != kStruct, "type should be pointer to object for gea instruction");
    decltype(obj) result;
    result = obj->Struct[g->GetElement()];
    valueMap_[g.get()] = result;
    return result;
  }

  Ref<Object> Execute(Ref<IdxInstruction> i) {
    auto obj = valueMap_[i->GetBase().get()];
    FAIL_IF(obj->Type != kPointer, "type should be pointer for idx instruction");
    // obj = obj->Pointer;
    // FAIL_IF(obj->Type != kArray, "type should be array for idx instruction");
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
    auto args_S = args_;

    std::vector<Ref<Object>> args;

    for (auto &arg : i->operands()) {
      auto obj = ConstantOrValue(arg);
      args.push_back(obj);
    }
    args_ = args;

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

    Loop();

    Ref<Object> result = nullptr;
    if (isReturn_)
      result = retResult_;

    next_ = next_S;
    current_ = current_S;
    isReturn_ = isReturn_S;
    args_ = args_S;
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
  std::vector<Ref<Object>> args_;
  size_t ticks_;
  bool isReturn_;
};

bool Interpreter::Execute(Ref<Module> m) {
  InterpreterImpl impl;
  return impl.Execute(m);
}

}
}
