#ifndef SEMANTIC_ERROR_H_
#define SEMANTIC_ERROR_H_

#include "jast/expression.h"
#include "jast/errors.h"
#include "jast/strings.h"

namespace jast {

class SemanticError : public Error {
public:
  SemanticError(std::string &&message, Handle<Expression> expr)
    : Error(std::move(message)), expr_{ expr }
  { }

  std::string Message() const override {
    return Strings::Format("SemanticError:{}:{} {}", expr_->loc().row(), expr_->loc().col(), Error::Message());
  }
private:
  Handle<Expression> expr_;
};

}

#endif
