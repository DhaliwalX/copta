#include "jast/ir/function.h"
#include "jast/ir/basic-block.h"
#include "jast/ir/instruction.h"
#include "jast/ir/value.h"

#include <iostream>

namespace jast {

void Function::print(std::ostream &os) const {
  os << name_ << "[";
  type_->dump();
  os << "]\n";
  for (auto &bb : bbs_) {
    bb->print(os);
    os << std::endl;
  }
  os << std::endl;
}

}
