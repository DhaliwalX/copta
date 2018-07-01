#include "jast/ir/value.h"
#include <iostream>


namespace jast {

void ConstantInt::print(std::ostream &os) const {
  os << value_;
}

void ConstantStr::print(std::ostream &os) const {
  os << value_;
}

void Void::print(std::ostream &os) const {
  os << "void";
}

void Parameter::print(std::ostream &os) const {
  os << "param@{";
  type_->dump();
  os << "}";
}

}