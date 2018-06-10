#include "jast/semantics/type-analysis.h"
#include "jast/strings.h"
#include <iostream>

namespace jast {


void TypeAnalysis::Visit(NullLiteral *literal)
{
}

void TypeAnalysis::Visit(UndefinedLiteral *literal)
{
}

void TypeAnalysis::Visit(ThisHolder *holder)
{
}

void TypeAnalysis::Visit(IntegralLiteral *literal)
{
}

void TypeAnalysis::Visit(StringLiteral *literal)
{
}

void TypeAnalysis::Visit(TemplateLiteral *literal)
{
}

void TypeAnalysis::Visit(ArrayLiteral *literal)
{

}

void TypeAnalysis::Visit(ObjectLiteral *literal)
{

}

void TypeAnalysis::Visit(Identifier *id)
{
}

void TypeAnalysis::Visit(BooleanLiteral *literal)
{
}

void TypeAnalysis::Visit(RegExpLiteral *reg)
{
}

void TypeAnalysis::Visit(ArgumentList *args)
{
}

void TypeAnalysis::Visit(CallExpression *expr)
{
}

void TypeAnalysis::Visit(MemberExpression *expr)
{
}

void TypeAnalysis::Visit(NewExpression *expr)
{
}

void TypeAnalysis::Visit(PrefixExpression *expr)
{
}

void TypeAnalysis::Visit(PostfixExpression *expr)
{
}

void TypeAnalysis::Visit(BinaryExpression *expr)
{
}

void TypeAnalysis::Visit(AssignExpression *expr)
{
  Type *t = detector_.Detect(expr->rhs());
  if (t == nullptr) {
    return;
  }

  Type *l = detector_.Detect(expr->lhs());
  if (l->IsUnresolvedType()) {
    auto r = dynamic_cast<UnresolvedType*>(l);
    if (!r->IsResolved() &&
        (!t->IsUnresolvedType() ||
            t->AsUnresolvedType()->IsResolved())) {
      r->ResolveTo(t);
      return;
    }
  }

  if (!l->Equals(t)) {
    err(Strings::Raw("Type mismatch in assignment"), expr->AsAssignExpression());
  }
}

void TypeAnalysis::Visit(TernaryExpression *expr)
{
}

void TypeAnalysis::Visit(CommaExpression *expr)
{
}

void TypeAnalysis::Visit(Declaration *decl)
{
}

void TypeAnalysis::Visit(DeclarationList *decl_list)
{
  for (auto &decl : decl_list->exprs()) {
    Type *t = detector_.Detect(decl);
    (void)t;
  }
}

void TypeAnalysis::Visit(BlockStatement *stmt)
{
  detector_.OpenScope();
  for (auto &st : *(stmt->statements())) {
    st->Accept(this);
  }
  detector_.CloseScope();
}

void TypeAnalysis::Visit(ForStatement *stmt)
{
}

void TypeAnalysis::Visit(WhileStatement *stmt)
{
}

void TypeAnalysis::Visit(DoWhileStatement *stmt)
{
}

void TypeAnalysis::Visit(BreakStatement *stmt)
{
}

void TypeAnalysis::Visit(ContinueStatement *stmt)
{
}

void TypeAnalysis::Visit(ThrowStatement *stmt)
{
}

void TypeAnalysis::Visit(TryCatchStatement *stmt)
{
}

void TypeAnalysis::Visit(LabelledStatement *stmt)
{
}

void TypeAnalysis::Visit(CaseClauseStatement *stmt)
{
}

void TypeAnalysis::Visit(SwitchStatement *stmt)
{
}

void TypeAnalysis::Visit(FunctionPrototype *proto)
{
}

void TypeAnalysis::Visit(FunctionStatement *stmt)
{
}

void TypeAnalysis::Visit(IfStatement *stmt)
{
}

void TypeAnalysis::Visit(IfElseStatement *stmt)
{
}

void TypeAnalysis::Visit(ReturnStatement *stmt)
{
}

void TypeAnalysis::dump() {
  for (auto &err : Errors()) {
    std::cout << err.Message() << std::endl;
  }

  for (auto &err : detector_.Errors()) {
    std::cout << err.Message() << std::endl;
  }
}

}
