#ifndef STATEMENT_H_
#define STATEMENT_H_

#include "jast/expression.h"

#include <iostream>
namespace jast {

///  BlockStatment ::= class representing block statments
class BlockStatement : public Expression {
public:
    BlockStatement(Position &loc, Scope *scope, Handle<ExpressionList> stmts)
        : Expression(loc, scope), stmts_{ stmts }
    { }


    Handle<ExpressionList> statements() { return stmts_; }
    void PushExpression(Handle<Expression> expr);
    DEFINE_NODE_TYPE(BlockStatement);
private:
    Handle<ExpressionList> stmts_;
};

enum class ForKind {
    kForOf,
    kForIn
};

class ForStatement : public Expression {
public:
    ForStatement(Position &loc, Scope *scope, ForKind kind, Handle<Expression> init,
        Handle<Expression> condition, Handle<Expression> update, Handle<Expression> body)
        : Expression(loc, scope), kind_{ kind }, init_{ init }, condition_{ condition },
          update_{ update }, body_{ body }
    { }


    Handle<Expression> init() { return init_; }
    Handle<Expression> condition() { return condition_; }
    Handle<Expression> update() { return update_; }
    Handle<Expression> body() { return body_; }

    ForKind kind() { return kind_; }

    DEFINE_NODE_TYPE(ForStatement);
private:
    ForKind kind_;
    Handle<Expression> init_;
    Handle<Expression> condition_;
    Handle<Expression> update_;
    Handle<Expression> body_;
};

class WhileStatement : public Expression {
    using ExprPtr = Handle<Expression>; // just for convenience
public:
    WhileStatement(Position &loc, Scope *scope, ExprPtr condition, ExprPtr body)
        : Expression(loc, scope), condition_{ condition },
          body_{ body }
    { }


    ExprPtr condition() { return condition_; }
    ExprPtr body() { return body_; }

    DEFINE_NODE_TYPE(WhileStatement);
private:
    Handle<Expression> condition_;
    Handle<Expression> body_;
};

class DoWhileStatement : public Expression {
    using ExprPtr = Handle<Expression>;
    DEFINE_NODE_TYPE(DoWhileStatement);
public:
    DoWhileStatement(Position &loc, Scope *scope, ExprPtr condition, ExprPtr body)
        : Expression(loc, scope), condition_{ condition },
          body_{ body }
    { }

    ExprPtr condition() { return condition_; }
    ExprPtr body() { return body_; }
private:
    Handle<Expression> condition_;
    Handle<Expression> body_;
};

// FunctionPrototype - captures the prototype of the function
// which includes the name of the function and
class FunctionPrototype : public Expression {
    DEFINE_NODE_TYPE(FunctionPrototype);
public:
    FunctionPrototype(Position &loc, Scope *scope, std::string name,
        std::vector<std::string> args, Type *type)
        : Expression(loc, scope), name_{ name }, args_{ std::move(args) }, type_(type)
    { }
    const std::string &GetName() const;
    const std::vector<std::string> &GetArgs() const;
    Type *GetType() { return type_;}
private:
    std::string name_;
    std::vector<std::string> args_;
    Type *type_;
};

// FunctionStatement - captures the function statement
class FunctionStatement : public Expression {
    DEFINE_NODE_TYPE(FunctionStatement);
public:
    FunctionStatement(Position &loc, Scope *scope,
        Handle<FunctionPrototype> proto, Handle<Expression> body)
        : Expression(loc, scope), proto_{ (proto) }, body_{ body }
    { }


    Handle<FunctionPrototype> proto() { return proto_; }
    Handle<Expression> body() { return body_; }
private:
    Handle<FunctionPrototype> proto_;
    Handle<Expression> body_;
};

class IfStatement : public Expression {
    DEFINE_NODE_TYPE(IfStatement);
    using ExprPtr = Handle<Expression>;
public:
    IfStatement(Position &loc, Scope *scope, ExprPtr cond, ExprPtr body)
        : Expression(loc, scope), condition_{ (cond) }, body_{ (body) }
    { }


    Handle<Expression> condition() { return condition_; }
    Handle<Expression> body() { return body_; }
private:
    Handle<Expression> condition_;
    Handle<Expression> body_;
};

class IfElseStatement : public Expression {
    DEFINE_NODE_TYPE(IfElseStatement);
    using ExprPtr = Handle<Expression>;
public:
    IfElseStatement(Position &loc, Scope *scope, ExprPtr cond, ExprPtr body, ExprPtr el)
    : Expression(loc, scope), condition_{ (cond) },
      body_{ (body) },
      else_{ (el) }
    { }


    Handle<Expression> condition() { return condition_; }
    Handle<Expression> body() { return body_; }
    Handle<Expression> els() { return else_; }
private:
    Handle<Expression> condition_;
    Handle<Expression> body_;
    Handle<Expression> else_;
};

class ReturnStatement : public Expression {
public:
    ReturnStatement(Position &loc, Scope *scope, Handle<Expression> expr)
        : Expression(loc, scope), expr_{ (expr) }
    { }

    Handle<Expression> expr() { return expr_; }

    DEFINE_NODE_TYPE(ReturnStatement);
private:
    Handle<Expression> expr_;
};

}

#endif
