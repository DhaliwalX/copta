#include "jast/semantics/symtab-generator.h"
#include "jast/strings.h"

namespace jast {

void SymTabGenerator::Visit(NullLiteral *literal) {}
void SymTabGenerator::Visit(UndefinedLiteral *literal) {}
void SymTabGenerator::Visit(ThisHolder *this) {}

void SymTabGenerator::Visit(IntegralLiteral *) {}

void SymTabGenerator::Visit(StringLiteral *) {}


void SymTabGenerator::Visit(TemplateLiteral *literal)
{
}

void SymTabGenerator::Visit(ArrayLiteral *literal)
{
  for (auto &expr : literal->exprs()) {
    visit(expr);
  }
}

void SymTabGenerator::Visit(ObjectLiteral *literal)
{
  for (auto &prop : literal->proxy()) {
    visit(p.second);
  }
}

void SymTabGenerator::Visit(Identifier *id)
{
  auto sym = table_.GetSymbol(id->GetName());
  if (!sym) {
    err(Strings::Format("{} not defined", id->GetName()));
  }
}

void SymTabGenerator::Visit(BooleanLiteral *literal)
{
}

void SymTabGenerator::Visit(RegExpLiteral *reg)
{
}

void SymTabGenerator::Visit(ArgumentList *args)
{
    for (auto &expr : args->args()) {
      visit(expr);
    }
}

void SymTabGenerator::Visit(CallExpression *expr)
{
    visit(expr->expr());
    switch (expr->kind()) {
    case MemberAccessKind::kCall:
        visit(expr->member())
        break;
    case MemberAccessKind::kDot:
        visit(expr->member());
        break;
    case MemberAccessKind::kIndex:
        visit(expr->member());
        break;
    default:
        throw std::runtime_error("");
    }
}

void SymTabGenerator::Visit(MemberExpression *expr)
{
    visit(expr->expr());
    switch (expr->kind()) {
    case MemberAccessKind::kCall:
        visit(expr->member())
        break;
    case MemberAccessKind::kDot:
        visit(expr->member());
        break;
    case MemberAccessKind::kIndex:
        visit(expr->member());
        break;
    default:
        throw std::runtime_error("");
    }
}

void SymTabGenerator::Visit(NewExpression *expr)
{
    visit(expr->member());
}

void SymTabGenerator::Visit(PrefixExpression *expr)
{
    os() << "PrefixExpression {\n";
    tab()++;
    os_tabbed() << "OP(";
    switch (expr->op()) {
    case PrefixOperation::kIncrement:
        os() << "++";
        break;
    case PrefixOperation::kDecrement:
        os() << "--";
        break;
    case PrefixOperation::kTypeOf:
        os() << "typeof ";
        break;
    case PrefixOperation::kDelete:
        os() << "delete ";
        break;
    case PrefixOperation::kBitNot:
        os() << "~";
        break;
    case PrefixOperation::kNot:
        os() << "!";
        break;
    case PrefixOperation::kVoid:
        os() << "void ";
        break;
    default:
        throw std::runtime_error("invalid prefix operator");
    }
    os() << ")\n";
    os_tabbed() << "Expression {\n";
    tab()++;
    os_tabbed();
    expr->expr()->Accept(this);
    tab()--;
    os_tabbed() << "}\n";

    tab()--;
    os_tabbed() << "}\n";
}

void SymTabGenerator::Visit(PostfixExpression *expr)
{
    os() << "PostfixExpression {\n";
    tab()++;
    os_tabbed();
    expr->expr()->Accept(this);

    os_tabbed() << "OP(";
    switch (expr->op()) {
    case PostfixOperation::kIncrement:
        os() << "++";
        break;
    case PostfixOperation::kDecrement:
        os() << "--";
        break;
    default:
        throw std::runtime_error("invalid postfix operator");
    }

    os() << ")\n";
    tab()--;
    os_tabbed() << "}\n";
}

void SymTabGenerator::Visit(BinaryExpression *expr)
{
    os() << "BinaryExpression {\n";
    tab()++;
    os_tabbed();
    expr->lhs()->Accept(this);
    os_tabbed() << "OP(";
    switch (expr->op()) {
    case BinaryOperation::kAddition:
        os() << "+";
        break;
    case BinaryOperation::kMultiplication:
        os() << "*";
        break;
    case BinaryOperation::kSubtraction:
        os() << "-";
        break;
    case BinaryOperation::kDivision:
        os() << "/";
        break;
    case BinaryOperation::kMod:
        os() << "%";
        break;
    case BinaryOperation::kShiftRight:
        os() << ">>";
        break;
    case BinaryOperation::kShiftZeroRight:
        os() << ">>>";
        break;
    case BinaryOperation::kShiftLeft:
        os() << "<<";
        break;
    case BinaryOperation::kLessThan:
        os() << "<";
        break;
    case BinaryOperation::kGreaterThan:
        os() << ">";
        break;
    case BinaryOperation::kLessThanEqual:
        os() << "<=";
        break;
    case BinaryOperation::kGreaterThanEqual:
        os() << ">=";
        break;
    case BinaryOperation::kEqual:
        os() << "==";
        break;
    case BinaryOperation::kNotEqual:
        os() << "!=";
        break;
    case BinaryOperation::kStrictEqual:
        os() << "===";
        break;
    case BinaryOperation::kStrictNotEqual:
        os() << "!==";
        break;
    case BinaryOperation::kAnd:
        os() << "&&";
        break;
    case BinaryOperation::kOr:
        os() << "||";
        break;
    case BinaryOperation::kBitAnd:
        os() << "&";
        break;
    case BinaryOperation::kBitOr:
        os() << "|";
        break;
    case BinaryOperation::kBitXor:
        os() << "^";
        break;
    case BinaryOperation::kInstanceOf:
        os() << "instanceof";
        break;
    case BinaryOperation::kIn:
        os() << "in";
        break;
    default: throw std::runtime_error("invalid binary operation");
    }
    os() << ")\n";
    os_tabbed();
    expr->rhs()->Accept(this);
    tab()--;
    os_tabbed();
    os() << "}\n";
}

void SymTabGenerator::Visit(AssignExpression *expr)
{
    os() << "AssignExpression {\n";
    tab()++;
    os_tabbed();
    expr->lhs()->Accept(this);
    os_tabbed();
    expr->rhs()->Accept(this);
    tab()--;
    os_tabbed() << "}\n";
}

void SymTabGenerator::Visit(TernaryExpression *expr)
{
    os() << "TernaryExpression {\n";
    tab()++;
    os_tabbed();
    expr->first()->Accept(this);
    os_tabbed();
    expr->second()->Accept(this);
    os_tabbed();
    expr->third()->Accept(this);
    tab()--;
    os_tabbed() << "}\n";
}

void SymTabGenerator::Visit(CommaExpression *expr)
{
    os() << "CommaExpression {\n";
    tab()++;
    auto exprs = expr->exprs();
    auto b = exprs->begin();
    auto e = exprs->end();

    for (; b != e; ++b) {
        os_tabbed();
        (*b)->Accept(this);
    }

    tab()--;
    os_tabbed() << "}\n";
}

void SymTabGenerator::Visit(Declaration *decl)
{
    os() << "Declaration (" << decl->name() << ") {\n";

    if (decl->expr()) {
        tab()++;
        os_tabbed();
        decl->expr()->Accept(this);
        tab()--;
    }
    os_tabbed() << "}\n";
}

void SymTabGenerator::Visit(DeclarationList *decl_list)
{
    os() << "DeclarationList {\n";
    auto &list = decl_list->exprs();

    tab()++;
    for (auto &decl : list) {
        os_tabbed();
        decl->Accept(this);
    }
    tab()--;
    os_tabbed() << "}\n";
}

void SymTabGenerator::Visit(BlockStatement *stmt)
{
    os() << "BlockStatement {\n";
    auto list = stmt->statements();

    tab()++;
    for (auto &expr : *list) {
        os_tabbed();
        expr->Accept(this);
    }
    tab()--;
    os_tabbed() << "}\n";
}

void SymTabGenerator::Visit(ForStatement *stmt)
{
    os() << "ForStatement {\n";
    tab()++;
    if (stmt->init()) {
        os_tabbed() << "ForInit {\n";
        tab()++;
        os_tabbed();
        stmt->init()->Accept(this);
        tab()--;
        os_tabbed();
        os() << "}\n";
    }

    if (stmt->condition()) {
        os_tabbed() << "ForCondition {\n";
        tab()++;
        os_tabbed();
        stmt->condition()->Accept(this);
        tab()--;
        os_tabbed() << "}\n";
    }

    if (stmt->update()) {
        os_tabbed() << "ForUpdate {\n";
        tab()++;
        os_tabbed();
        stmt->update()->Accept(this);
        tab()--;
        os_tabbed() << "}\n";
    }

    os_tabbed() << "ForBody {\n";
    tab()++;
    os_tabbed();
    stmt->body()->Accept(this);
    tab()--;
    os_tabbed() << "}\n";
    tab()--;
    os_tabbed() << "}\n";
}

void SymTabGenerator::Visit(WhileStatement *stmt)
{
    os() << "WhileStatement {\n";
    tab()++;
    os_tabbed() << "WhileCondition {\n";
    tab()++;
    os_tabbed();
    stmt->condition()->Accept(this);
    tab()--;
    os_tabbed() << "}\n";
    os_tabbed() << "WhileBody {\n";

    tab()++;
    os_tabbed();
    stmt->body()->Accept(this);
    tab()--;
    os_tabbed() << "}\n";
    tab()--;
    os_tabbed() << "}\n";
}

void SymTabGenerator::Visit(DoWhileStatement *stmt)
{
    os() << "DoWhileStatement {\n";
    tab()++;

    os_tabbed() << "DoWhileBody {\n";
    tab()++;
    os_tabbed();
    stmt->body()->Accept(this);
    tab()--;
    os_tabbed() << "}\n";
    os_tabbed() << "DoWhileCondition {\n";
    tab()++;
    os_tabbed();
    stmt->condition()->Accept(this);
    tab()--;
    os_tabbed() << "}\n";
    tab()--;
    os_tabbed() << "}\n";
}

void SymTabGenerator::Visit(BreakStatement *stmt)
{
    os() << "BreakStatement";
    if (stmt->label()) {
        os() << " (label=";
        stmt->label()->Accept(this);
        os() << ")";
    }
    os() << "\n";
}

void SymTabGenerator::Visit(ContinueStatement *stmt)
{
    os() << "ContinueStatement";
    if (stmt->label()) {
        os() << " (label=";
        stmt->label()->Accept(this);
        os() << ")";
    }
    os() << "\n";
}

void SymTabGenerator::Visit(ThrowStatement *stmt)
{
    os() << "ThrowStatement {\n";
    tab()++;
    os_tabbed();
    stmt->expr()->Accept(this);
    tab()--;
    os_tabbed() << "}\n";
}

void SymTabGenerator::Visit(TryCatchStatement *stmt)
{
    os() << "TryCatchStatement {\n";
    tab()++;
    os_tabbed() << "TryBlock {\n";
    tab()++;
    os_tabbed();
    stmt->try_block()->Accept(this);
    tab()--;
    os_tabbed() << "}\n";
    if (stmt->catch_expr()) {
        os_tabbed() << "CatchExpression (e=" << stmt->catch_expr()->AsIdentifier()->GetName();
        os() << ") {\n";
        tab()++;
        os_tabbed();
        stmt->catch_block()->Accept(this);
        tab()--;
        os_tabbed() << "}\n";
    }

    if (stmt->finally()) {
        os_tabbed() << "FinallyExpression {\n";
        tab()++;
        os_tabbed();
        stmt->finally()->Accept(this);
        tab()--;
        os_tabbed() << "}\n";
    }
    tab()--;
    os_tabbed() << "}\n";
}

void SymTabGenerator::Visit(LabelledStatement *stmt)
{
    os() << "LabelledStatement (label=" << stmt->label() << ")\n";
}

void SymTabGenerator::Visit(CaseClauseStatement *stmt)
{
    os() << "CaseClauseStatement {\n";
    tab()++;
    os_tabbed() << "CaseClause {\n";
    tab()++;
    os_tabbed();
    stmt->clause()->Accept(this);
    tab()--;
    os_tabbed() << "}\n";
    os_tabbed() << "Statement {\n";
    tab()++;
    os_tabbed();
    stmt->stmt()->Accept(this);
    tab()--;
    os_tabbed() << "}\n";
    tab()--;
    os_tabbed() << "}\n";
}

void SymTabGenerator::Visit(SwitchStatement *stmt)
{
    os() << "SwitchStatement {\n";

    tab()++;
    os_tabbed() << "SwitchExpression {\n";
    tab()++;
    stmt->expr()->Accept(this);
    tab()--;
    os_tabbed() << "}\n";

    os_tabbed() << "SwitchBlock {\n";
    tab()++;
    auto list = stmt->clauses();

    for (auto &clause : *list) {
        os_tabbed();
        clause->Accept(this);
    }

    if (stmt->default_clause()) {
        os_tabbed() << "DefaultClause {\n";
        tab()++;
        os_tabbed();
        stmt->default_clause()->Accept(this);
        tab()--;
        os_tabbed() << "}\n";
    }
    tab()--;
    os_tabbed() << "}\n";
    tab()--;
    os_tabbed() << "}\n";
}

void SymTabGenerator::Visit(FunctionPrototype *proto)
{
    os() << "FunctionPrototype (name=" << proto->GetName() << ") {\n";

    tab()++;
    os_tabbed() << "Arguments (";
    std::string out;
    for (const auto &arg : proto->GetArgs()) {
        out += arg + ", ";
    }

    if (proto->GetArgs().size()) {
        out.pop_back();
        out.pop_back();
        os() << out;
    }
    os() << ")\n";
    tab()--;
    os_tabbed() << "}\n";
}

void SymTabGenerator::Visit(FunctionStatement *stmt)
{
    os() << "FunctionStatement {\n";
    tab()++;
    os_tabbed();
    stmt->proto()->Accept(this);
    os_tabbed() << "FunctionBody {\n";
    tab()++;
    os_tabbed();
    stmt->body()->Accept(this);
    tab()--;
    os_tabbed() << "}\n";
    tab()--;
    os_tabbed() << "}\n";
}

void SymTabGenerator::Visit(IfStatement *stmt)
{
    os() << "IfStatement {\n";
    tab()++;
    os_tabbed() << "IfCondition {\n";
    tab()++;
    os_tabbed();
    stmt->condition()->Accept(this);
    tab()--;
    os_tabbed() << "}\n";
    os_tabbed() << "TrueBody {\n";
    tab()++;
    os_tabbed();
    stmt->body()->Accept(this);
    tab()--;
    os_tabbed() << "}\n";
    tab()--;
    os_tabbed() << "}\n";
}

void SymTabGenerator::Visit(IfElseStatement *stmt)
{
    os() << "IfStatement {\n";
    tab()++;
    os_tabbed() << "IfCondition {\n";
    tab()++;
    os_tabbed();
    stmt->condition()->Accept(this);
    tab()--;
    os_tabbed() << "}\n";
    os_tabbed() << "TrueBody {\n";
    tab()++;
    os_tabbed();
    stmt->body()->Accept(this);
    tab()--;

    os_tabbed() << "FalseBody {\n";
    tab()++;
    os_tabbed();
    stmt->els()->Accept(this);
    tab()--;
    os_tabbed() << "}\n";
    tab()--;
    os_tabbed() <<"}\n";
}

void SymTabGenerator::Visit(ReturnStatement *stmt)
{
    os() << "ReturnStatement";
    if (stmt->expr()) {
        os() << " {\n";
        tab()++;
        os_tabbed();
        stmt->expr()->Accept(this);
        tab()--;
        os_tabbed() << "}";
    }
    os() << "\n";
}

}
