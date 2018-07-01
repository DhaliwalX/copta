#ifndef MODULE_H_
#define MODULE_H_

#include "jast/ir/function.h"

#include <string>
#include <map>
namespace jast {

class Module : public RefCountObject {
public:
  Module() = default;
  void Add(std::string name, Ref<Function> fun) {
    functions_[name] = fun;
  }


  std::map<std::string, Ref<Function>> &functions() {
    return functions_;
  }

  void dump(std::ostream &os) const;

private:
  std::map<std::string, Ref<Function>> functions_;
};

}

#endif
