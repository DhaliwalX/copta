#ifndef TYPE_DETECTOR_H_
#define TYPE_DETECTOR_H_

#include "jast/types/type.h"
#include "jast/expression.h"
#include "jast/statement.h"
#include "jast/semantics/error.h"

namespace jast {

class TypeDetectorImpl;

class TypeDetector {
public:
  TypeDetector();
  ~TypeDetector();
  Type *Detect(Handle<Expression> expr);

  std::vector<SemanticError> &Errors();

  void OpenScope();
  void CloseScope();
private:
  TypeDetectorImpl *impl_;
};

}

#endif
