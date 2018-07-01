#include "jast/semantics/type-analysis.h"
#include "jast/strings.h"
#include <iostream>

namespace jast {

void TypeAnalysis::setCurrentFunctionType(FunctionType *t) {
  currentFunctionStack.push_back(t);
}

FunctionType *TypeAnalysis::currentFunctionType() {
  return currentFunctionStack.back();
}

void TypeAnalysis::removeCurrentFunctionType() {
  currentFunctionStack.pop_back();
}

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
  Type *rhs = detector_.Detect(expr->rhs());
  Type *lhs = detector_.Detect(expr->lhs());
  if (rhs != lhs) {
    err(Strings::Raw("Type mismatch in assignment"), expr->AsAssignExpression());
  }
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
  auto proto = stmt->proto();
  FunctionType *t = proto->GetType()->AsFunctionType();
  detector_.AddSymbol(proto->GetName(), t);

  setCurrentFunctionType(t);
  // function scope starts
  detector_.OpenScope();
  auto &args = proto->GetArgs();
  int i = 0;
  for (auto arg_type : t->getArgumentsTypes()) {
    detector_.AddSymbol(args[i++], arg_type);
  }

  visit(stmt->body());
  detector_.CloseScope();
  removeCurrentFunctionType();
}

void TypeAnalysis::Visit(IfStatement *stmt)
{
  auto t = detector_.Detect(stmt->condition());
  if (!t->IsIntegerType()) {
    err(Strings::Format("condition should be an integer type"), stmt->condition());
  }

  visit(stmt->body());
}

void TypeAnalysis::Visit(IfElseStatement *stmt)
{
  auto t = detector_.Detect(stmt->condition());
  if (!t->IsIntegerType()) {
    err(Strings::Format("condition should be an integer type"), stmt->condition());
  }

  visit(stmt->body());
  visit(stmt->els());
}

void TypeAnalysis::Visit(ReturnStatement *stmt)
{
  auto t = detector_.Detect(stmt->expr());
  auto curr = currentFunctionType();
  auto ret_type = curr->getReturnType();
  if (ret_type != t) {
    err(Strings::Format("return type mismatch"), stmt->expr());
  }
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
