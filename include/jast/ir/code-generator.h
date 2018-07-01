#ifndef CODE_GENERATOR_H_
#define CODE_GENERATOR_H_

#include "jast/handle.h"
#include "jast/ir/value.h"
#include "jast/ir/instruction.h"
#include "jast/ir/basic-block.h"
#include "jast/ir/function.h"
#include "jast/ir/module.h"

namespace jast {
class Expression;
class CodeGeneratorImpl;

class CodeGenerator {
public:
  CodeGenerator();
  ~CodeGenerator();
  Ref<Module> RunOn(Ref<Expression> ast);
private:
  CodeGeneratorImpl *impl_;

  DISABLE_COPY(CodeGenerator);
};

}

#endif
