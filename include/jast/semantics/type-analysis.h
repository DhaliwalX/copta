#ifndef TYPE_ANALYSIS_H_
#define TYPE_ANALYSIS_H_

#include "jast/astvisitor.h"
#include "jast/semantics/type-detector.h"
#include "jast/semantics/error.h"

namespace jast {

class TypeAnalysis : public ASTVisitor {
public:
  TypeAnalysis() = default;

#define DECLARE_VISITOR_METHOD(type) void Visit(jast::type *) override;
AST_NODE_LIST(DECLARE_VISITOR_METHOD)
#undef DECLARE_VISITOR_METHOD

  void err(std::string &&message, Handle<Expression> expr) {
    errors_.push_back(SemanticError(std::forward<std::string>(message), expr));
  }

  std::vector<SemanticError> &Errors() {
    return errors_;
  }

  void dump();
private:
  std::vector<SemanticError> errors_;
  TypeDetector detector_;
};

}

#endif
