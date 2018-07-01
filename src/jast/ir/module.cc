#include "jast/ir/module.h"
#include "jast/ir/value.h"
#include "jast/ir/instruction.h"
#include "jast/ir/basic-block.h"
#include "jast/ir/function.h"

#include <iostream>

namespace jast {

void Module::dump(std::ostream &os) const {
  os << "module" << std::endl;
  os << "\n";

  for (auto &function : functions_) {
    function.second->print(os);
    os << "\n\n";
  }
}

}
