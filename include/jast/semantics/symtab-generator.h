#ifndef SYMTAB_GENERATOR_H_
#define SYMTAB_GENERATOR_H_

#include "jast/astvisitor.h"
#include "jast/semantics/symbol-table.h"
#include "jast/errors.h"

namespace jast {

class SymTabGenerator : public ASTVisitor {
public:
#define DECLARE_VISITOR_METHOD(type) void Visit(jast::type *) override;
AST_NODE_LIST(DECLARE_VISITOR_METHOD)
#undef DECLARE_VISITOR_METHOD

  void err(std::string &&message) {
    errors_.emplace_back(std::forward(message));
  }

  void warn(std::string &&message) {
    warnings_.emplace_back(std::forward(message));
  }
private:
  SymbolTable table_;
  std::vector<Error> errors_;
  std::vector<Warning> warnings_;
};

}

#endif
