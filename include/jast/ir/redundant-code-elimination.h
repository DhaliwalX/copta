#ifndef REDUNDANT_CODE_ELIMINATION_H_
#define REDUNDANT_CODE_ELIMINATION_H_

#include "jast/ir/module.h"
#include "jast/ir/function.h"
#include "jast/ir/basic-block.h"
#include "jast/ir/instruction.h"
#include "jast/ir/value.h"

namespace jast {

static bool HasSideEffect(Ref<Instruction> i) {
  switch (i->opCode()) {
    case OpCode::kStore:
    case OpCode::kInvoke:
    case OpCode::kRet:
    case OpCode::kBrk:
    case OpCode::kJmp:
      return true;

    default:
      return false;
  }
}

class RedundantCodeEliminator {
public:
  bool RunOn(Ref<Module> m) {
    for (auto &f : m->functions()) {
      if (!RunOn(f.second))
        return false;
    }
    return true;
  }

  bool RunOn(Ref<Function> f) {
    for (auto &bb : *f) {
      if (!RunOn(bb)) {
        return false;
      }
    }

    return true;
  }

  bool RunOn(Ref<BasicBlock> b) {

    for (auto i = b->begin(); i != b->end();) {
      auto next = i++;
      if (HasSideEffect(*next)) {
        continue;
      }

      if ((*next)->user_list().empty()) {
        RemoveInstruction(b, next);
      }
    }

    return true;
  }
};

}

#endif
