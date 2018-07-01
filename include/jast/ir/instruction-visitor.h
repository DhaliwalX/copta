#ifndef INSTRUCTION_VISITOR_H_
#define INSTRUCTION_VISITOR_H_

#include "jast/ir/instruction.h"

namespace jast {

template <typename RetType>
class InstructionVisitor {
public:
  template <OpCode op>
  RetType Visit(Ref<BinaryInstruction<op>> b);

  RetType Visit(Ref<AllocInstruction> a);
  RetType Visit(Ref<LoadInstruction> l);
  RetType Visit(Ref<StoreInstruction> s);
  RetType Visit(Ref<BrkInstruction> b);
  RetType Visit(Ref<JmpInstruction> j);
  RetType Visit(Ref<GeaInstruction> g);
  RetType Visit(Ref<IdxInstruction> i);
  RetType Visit(Ref<RetInstruction> r);
  RetType Visit(Ref<InvokeInstruction> c);
};

}

#endif