#include "jast/semantics/type-analysis.h"
#include "jast/strings.h"
#include <iostream>

namespace jast {


void TypeAnalysis::Visit(NullLiteral *literal)
{
  // nothing
}

void TypeAnalysis::Visit(ThisHolder *holder)
{
  // nothing
}

void TypeAnalysis::Visit(IntegralLiteral *literal)
{
  // nothing
}

void TypeAnalysis::Visit(StringLiteral *literal)
{
  // nothing
}

void TypeAnalysis::Visit(ArrayLiteral *literal)
{
  // nothing
}

void TypeAnalysis::Visit(ObjectLiteral *literal)
{
  // nothing
}

void TypeAnalysis::Visit(Identifier *id)
{
  // nothing
}

void TypeAnalysis::Visit(BooleanLiteral *literal)
{
  // nothing
}

void TypeAnalysis::Visit(ArgumentList *args)
{
  detector_.Detect(args->AsArgumentList());
}

void TypeAnalysis::Visit(CallExpression *expr)
{
  detector_.Detect(expr->AsCallExpression());
}

void TypeAnalysis::Visit(MemberExpression *expr)
{
  detector_.Detect(expr->AsMemberExpression());
}

void TypeAnalysis::Visit(PrefixExpression *expr)
{
  detector_.Detect(expr->AsPrefixExpression());
}

void TypeAnalysis::Visit(PostfixExpression *expr)
{
  detector_.Detect(expr->AsPostfixExpression());
}

void TypeAnalysis::Visit(BinaryExpression *expr)
{
  detector_.Detect(expr->AsBinaryExpression());
}

void TypeAnalysis::Visit(AssignExpression *expr)
{
  detector_.Detect(expr->AsAssignExpression());
}

void TypeAnalysis::Visit(Declaration *decl)
{
  detector_.Detect(decl->AsDeclaration());
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
  detector_.OpenScope();
  if (stmt->init())
    stmt->init()->Accept(this);

  if (stmt->condition())
    stmt->condition()->Accept(this);

  if (stmt->body())
    stmt->body()->Accept(this);

  if (stmt->update())
    stmt->update()->Accept(this);

  detector_.CloseScope();
}

void TypeAnalysis::Visit(WhileStatement *stmt)
{
  if (stmt->condition())
    stmt->condition()->Accept(this);

  if (stmt->body())
    stmt->body()->Accept(this);
}

void TypeAnalysis::Visit(DoWhileStatement *stmt)
{
  if (stmt->condition())
    stmt->condition()->Accept(this);

  if (stmt->body())
    stmt->body()->Accept(this);
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
