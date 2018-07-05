#ifndef INTERP_H_
#define INTERP_H_

#include "jast/ir/module.h"
#include <iostream>

namespace jast {
namespace internal {

enum ObjectType {
  kUndefined,
  kNumber,
  kString,
  kArray,
  kStruct,
  kPointer,
};

// generic object
struct Object : public RefCountObject {
  ObjectType Type;
  int64_t Int;
  std::string Str;
  std::map<std::string, Ref<Object>> Struct;
  std::vector<Ref<Object>> Arr;
  Ref<Object> Pointer;


  Object(ObjectType type)
    : Type(type), Int(0), Str(), Struct(), Arr()
  { }
};

using NativeFunction = Ref<Object> (*) (std::vector<Ref<Object>> &args);

// store of native functions available to all the programs
class NativeFunctionResolver {
public:
  static NativeFunction Resolve(const std::string &name) {
    return functions_[name];
  }

  static void Add(const std::string &name, NativeFunction function) {
    functions_[name] = function;
    std::cout << "Registered " << name << " as native function\n";
  }
private:
  static std::map<std::string, NativeFunction> functions_;
};

class RegisterNativeFunction {
public:
  RegisterNativeFunction(const std::string &name, NativeFunction nf) {
    NativeFunctionResolver::Add(name, nf);
  }
};

// simple interpreter for testing validity of programs
// after running optimizations
class Interpreter {
public:
  bool Execute(Ref<Module> mod);
};

}
}

#endif
