#include "jast/semantics/type-detector.h"
#include "jast/semantics/symbol-table.h"
#include "jast/types/type-system.h"
#include "jast/strings.h"
#include "jast/semantics/error.h"

namespace jast {

class TypeDetectorImpl {
  friend class TypeDetector;
public:
  Type *TypeOf(Handle<Expression> expr) {
    switch (expr->type()) {
      case ASTNodeType::kIntegralLiteral:
        return TypeOfInteger(expr->AsIntegralLiteral());
      case ASTNodeType::kStringLiteral:
        return TypeOfString(expr->AsStringLiteral());
      case ASTNodeType::kDeclaration:
        return TypeOfDeclaration(expr->AsDeclaration());
      case ASTNodeType::kIdentifier:
        return TypeOfIdentifier(expr->AsIdentifier());
      case ASTNodeType::kArrayLiteral:
        return TypeOfArray(expr->AsArrayLiteral());
      case ASTNodeType::kBinaryExpression:
        return TypeOfBinaryExpression(expr->AsBinaryExpression());

      case ASTNodeType::kAssignExpression:
        return TypeOfAssignExpression(expr->AsAssignExpression());

      default:
        assert(0 && "Not implemented");
    }
  }

private:
  Type *TypeOfInteger(Handle<IntegralLiteral> literal) {
    return TypeSystem::getIntegerType();
  }

  Type *TypeOfString(Handle<StringLiteral> literal) {
    return TypeSystem::getStringType();
  }

  Type *TypeOfArray(Handle<ArrayLiteral> literal) {
    Type *last = nullptr;
    for (auto &expr : literal->exprs()) {
      Type *now = TypeOf(expr);
      if (last == nullptr) {
        last = now;
      } else if (last != now) {
        err(Strings::Raw("Type mismatch in array literal"), expr);
        return nullptr;
      }
    }

    return TypeSystem::getArrayType(last);
  }

  Type *TypeOfObject(Handle<ObjectLiteral> literal) {
    assert(false && "Not implemented");
  }

  Type *TypeOfIdentifier(Handle<Identifier> id) {
    auto sym = table_.GetSymbol(id->GetName());
    if (sym) {
      return sym->type();
    }

    err(Strings::Format("{} used before definition", id->GetName()), id);
  }

  Type *TypeOfBooleanLiteral(Handle<BooleanLiteral> literal) {
    return TypeSystem::getIntegerType();
  }

  Type *TypeOfCallExpression(Handle<CallExpression> expr) {
    Type *type = TypeOf(expr->expr());
    switch (expr->kind()) {
      case MemberAccessKind::kCall: {
        if (!type->IsFunctionType()) {
          err(Strings::Raw("not a function"), expr->expr());
          return nullptr;
        }
        FunctionType *t = type->AsFunctionType();
        return t->getReturnType();
      }

      case MemberAccessKind::kDot:
      {
        if (!type->IsObjectType()) {
          err(Strings::Raw("not an object"), expr->expr());
          return nullptr;
        }

        ObjectType *t = type->AsObjectType();
        auto propName = expr->member()->AsIdentifier()->GetName();
        Type *memberType = t->getMember(propName);
        if (memberType == nullptr) {
          err(Strings::Format("object does not have property named '{}'", propName), expr->member());
        }
        return memberType;
      }

      case MemberAccessKind::kIndex:
      {
        if (!type->IsArrayType()) {
          err(Strings::Raw("not an array"), expr->expr());
          return nullptr;
        }

        ArrayType *t = type->AsArrayType();
        return t->getBaseElementType();
      }

      default:
        err(Strings::Raw("unknown expression"), expr);
        return nullptr;
    }
  }

  Type *TypeOfMemberExpression(Handle<MemberExpression> expr) {
    Type *type = TypeOf(expr->expr());
    switch (expr->kind()) {
      case MemberAccessKind::kCall: {
        if (!type->IsFunctionType()) {
          err(Strings::Raw("not a function"), expr->expr());
          return nullptr;
        }
        FunctionType *t = type->AsFunctionType();
        return t->getReturnType();
      }

      case MemberAccessKind::kDot: {
        if (!type->IsObjectType()) {
          err(Strings::Format("not an object"), expr->expr());
          return nullptr;
        }

        ObjectType *t = type->AsObjectType();
        auto propName = expr->member()->AsIdentifier()->GetName();
        Type *memberType = t->getMember(propName);
        if (memberType == nullptr) {
          err(Strings::Format("object does not have property named '{}'", propName), expr->member());
        }
        return memberType;
      }

      case MemberAccessKind::kIndex: {
        if (!type->IsArrayType()) {
          err(Strings::Raw("not an array"), expr->expr());
          return nullptr;
        }

        ArrayType *t = type->AsArrayType();
        return t->getBaseElementType();
      }
      default:
        err(Strings::Raw("unknown expression"), expr);
        return nullptr;
    }
  }

  Type *TypeOfPrefixExpression(Handle<PrefixExpression> expr) {
    switch (expr->op()) {
      case PrefixOperation::kIncrement:
      case PrefixOperation::kDecrement:
      case PrefixOperation::kNot:
      case PrefixOperation::kBitNot: {
        Type *elementType = TypeOf(expr->expr());
        if (!elementType->IsIntegerType()) {
          err(Strings::Raw("numeric operation on non-numeric type"), expr);
          return nullptr;
        }

        return elementType;
      }
      default:
        return nullptr;
    }
  }

  Type *TypeOfPostfixExpression(Handle<PostfixExpression> expr) {
    Type *elementType = TypeOf(expr->expr());
    if (!elementType->IsIntegerType()) {
      err(Strings::Raw("numeric operation on non-numeric type"), expr);
      return nullptr;
    }

    return elementType;
  }

  Type *TypeOfBinaryExpression(Handle<BinaryExpression> expr) {
    Type *leftType = TypeOf(expr->lhs());
    Type *rightType = TypeOf(expr->rhs());

    switch (expr->op()) {
      case BinaryOperation::kAddition:
        if ((leftType->IsStringType() || leftType->IsIntegerType()) && (leftType == rightType)) {
          return leftType;
        }

        err(Strings::Raw("wrong type for binary operation"), expr);
        return nullptr;

      default:
        if (leftType->IsIntegerType() && leftType == rightType) {
          return leftType;
        }

        err(Strings::Raw("wrong type for binary operation"), expr);
    }
  }

  Type *TypeOfAssignExpression(Handle<AssignExpression> expr) {
    return TypeOf(expr->rhs());
  }

  Type *TypeOfDeclaration(Handle<Declaration> decl) {
    auto name = decl->name();
    auto sym = table_.GetSymbolInCurrentScope(name);
    if (sym) {
      err(Strings::Format("symbol ({}) is already defined", name), decl);
      return nullptr;
    }

    auto type = TypeSystem::getUnresolvedType();
    table_.PutSymbol(name, type);
    return type;
  }

  void err(std::string &&message, Handle<Expression> expr) {
    errs.push_back(SemanticError(std::forward<std::string>(message), expr));
  }

  void openScope() {
    table_.OpenScope();
  }

  void closeScope() {
    table_.CloseScope();
  }
private:
  std::vector<SemanticError> errs;
  SymbolTable table_;
};

TypeDetector::TypeDetector()
  : impl_{ new TypeDetectorImpl() }
{}

TypeDetector::~TypeDetector() {
  delete impl_;
}

void TypeDetector::OpenScope() {
  impl_->openScope();
}

void TypeDetector::CloseScope() {
  impl_->closeScope();
}

std::vector<SemanticError> &TypeDetector::Errors() {
  return impl_->errs;
}

Type *TypeDetector::Detect(Handle<Expression> expr) {
  return impl_->TypeOf(expr);
}

}
