#include "jast/ir/basic-block.h"
#include "jast/ir/instruction.h"
#include <iostream>

namespace jast {

void BasicBlock::print(std::ostream &os) const {
  os << label_ << ":\n";
  for (auto &i : instructions_) {
    os << "  ";
    i->print(os);
    os << std::endl;
  }
}

void BasicBlock::dump() const {
  print(std::cout);
}

}
