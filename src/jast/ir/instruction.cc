#include "jast/ir/instruction.h"
#include "jast/ir/function.h"
#include "jast/ir/basic-block.h"

#include <iostream>

namespace jast {

void Instruction::print(std::ostream &os) const {
  if (name_.length() > 0)
    os << name_ << " = " << IrToString(op_) << " ";
  else
    os << IrToString(op_) << " ";
  for (auto &op : operands_) {
    if (auto instr = dynamic_cast<const Instruction*>(op.get())) {
      instr->ShortPrint(os);
    } else if (auto bb = dynamic_cast<const BasicBlock*>(op.get())) {
      os << bb->label() << " ";
    } else {
      op->print(os);
    }
    os << " ";
  }

  os << " # " << use_list().size() << ":" << user_list().size();
}

void Instruction::ShortPrint(std::ostream &os) const {
  os << name_;
}

void AllocInstruction::print(std::ostream &os) const {
  os << name_ << " = alloc ";
  ((AllocInstruction*)this)->getType()->AsPointerType()->getBaseElementType()->dump();

  os << " # " << use_list().size() << ":" << user_list().size();
}

void InvokeInstruction::print(std::ostream &os) const {
  if (name_.length() > 0)
    os << name_ << " = " << IrToString(op_) << " ";
  else
    os << IrToString(op_) << " ";
  InvokeInstruction *i = ((InvokeInstruction*)this);
  os << i->GetFunction()->getName() << "(";
  for (const auto &arg : operands_) {
    if (auto instr = dynamic_cast<const Instruction*>(arg.get())) {
      instr->ShortPrint(os);
    } else {
      arg->print(os);
    }
    os << " ";
  }
  os << ")";

  os << " # " << use_list().size() << ":" << user_list().size();
}
}
