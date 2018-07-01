#include "jast/ir/debug-info.h"

#include <iostream>
#include <fmt/format.h>

namespace jast {

Position &DebugInfo::start() {
  return start_;
}

Position &DebugInfo::end() {
  return end_;
}

void DebugInfo::print(std::ostream &os) const {
  os << "@{" << start_.row() << ":" << start_.col() << "}-{"
      << end_.row() << ":" << end_.col() << "}";
}

void DebugInfo::dump() const {
  print(std::cout);
  std::cout << std::endl;
}

}
