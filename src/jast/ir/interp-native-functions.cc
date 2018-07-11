#include "jast/ir/interp-native-functions.h"

#include "jast/ir/interp.h"
#include "jast/ir/basic-block.h"
#include "jast/ir/function.h"
#include "jast/ir/instruction.h"
#include "jast/ir/module.h"

#include <iostream>

namespace jast {
namespace internal {

void Print(Ref<Object> &arg) {
  if (!arg) {
    std::cout << "{nullptr}";
    return;
  }
  switch (arg->Type) {
    case kNumber:
      std::cout << arg->Int;
      break;

    case kString:
      std::cout << arg->Str;
      break;

    case kStruct:
      std::cout << "{\n";
      for (auto &p : arg->Struct) {
        std::cout << p.first << ":";
        Print(p.second);
        std::cout << "\n";
      }
      std::cout << "}";
      break;

    case kPointer:
      std::cout << "&{";
      Print(arg->Pointer);
      std::cout << "}";
      break;

    case kUndefined:
      std::cout << "undef";
      break;

    case kArray:
      std::cout << "[";
      print(arg->Arr);
      std::cout << "]";
      break;
  }
}

Ref<Object> print(std::vector<Ref<Object>> &args) {
  if (args.size() < 1) {
    return nullptr;
  }
  auto it = args.begin();
  auto end = args.end() - 1;
  for (; it != end; ++it) {
    Print(*it);
    std::cout << ", ";
  }

  Print(*it);
  std::cout << "\n";
  return nullptr;
}


}
}
jast::internal::RegisterNativeFunction r("print", &jast::internal::print);
