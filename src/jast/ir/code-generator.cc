#include "jast/ir/code-generator.h"
#include "jast/ir/value.h"
#include "jast/ir/instruction.h"
#include "jast/ir/function.h"
#include "jast/ir/basic-block.h"
#include "jast/ir/ir-builder.h"
#include "jast/ir/module.h"

#include "jast/expression.h"
#include "jast/statement.h"
#include "jast/semantics/symbol-table.h"

namespace jast {

class SymbolTableManager : public RefCountObject {
public:
  SymbolDefinition *PutSymbol(const std::string &name, Type *type, Ref<Value> value) {
    return table_.PutSymbol(name, type, value);
  }

  SymbolDefinition *GetSymbol(const std::string &name) {
    return table_.GetSymbol(name);
  }

private:
  SymbolTable table_;
};

class TemporaryNameAssigner {
public:
  TemporaryNameAssigner()
    : counter_(0)
  { }

  std::string Name(Ref<Value> val) {
    auto it = known_names_.find(val.get());
    if (it == known_names_.end()) {
      return AssignName(val.get(), std::string("%") + std::to_string(counter_++));
    }

    return it->second;
  }

  std::string AssignName(Value *val, std::string name) {
    known_names_[val] = name;
    return name;
  }

private:
  int counter_;
  std::map<Value *, std::string> known_names_;
};

class NewBasicBlock {
public:
  NewBasicBlock(CodeGeneratorImpl *impl, const std::string &name = "");

  ~NewBasicBlock();
private:
  CodeGeneratorImpl *impl_;
  Ref<BasicBlock> bb_;
};

template <bool load>
class AddressEnv {
public:
  AddressEnv(CodeGeneratorImpl *impl);

  ~AddressEnv();
private:
  CodeGeneratorImpl *impl_;
  bool prev_;
};

class CodeGeneratorImpl {
  template <bool load>
  friend class AddressEnv;
  friend class NewBasicBlock;
  friend class NewFunction;
public:
  CodeGeneratorImpl(Ref<SymbolTableManager> manager)
    : manager_{manager}
  { }

  void RunOn(Ref<Expression> ast, Ref<Module> mod) {
    mod_ = mod;
    Ref<Function> function(new Function());
    function->setName("init");
    function->setType(TypeSystem::getFunctionType(TypeSystem::getUndefinedType(),
        {}));
    Ref<BasicBlock> bb(new BasicBlock(function, "entry"));
    function->AddBB(bb);
    builder_.SetFunction(function);
    builder_.SetBasicBlock(bb);
    RunOn(ast);
    mod_->Add("init", function);
  }

  Ref<Value> RunOn(Ref<Expression> ast) {
    switch (ast->type()) {
      case ASTNodeType::kNullLiteral:
        return nullptr;
      case ASTNodeType::kIntegralLiteral:
        return GenConstantInt(ast->AsIntegralLiteral());

      case ASTNodeType::kStringLiteral:
        return GenConstantStr(ast->AsStringLiteral());

      case ASTNodeType::kIdentifier:
        return GenIdentifier(ast->AsIdentifier());

      case ASTNodeType::kMemberExpression:
        return GenMemberExpression(ast->AsMemberExpression());

      case ASTNodeType::kPrefixExpression:
        return GenPrefixExpression(ast->AsPrefixExpression());

      case ASTNodeType::kPostfixExpression:
        return GenPostfixExpression(ast->AsPostfixExpression());

      case ASTNodeType::kBinaryExpression:
        return GenBinaryExpression(ast->AsBinaryExpression());

      case ASTNodeType::kAssignExpression:
        return GenAssignExpression(ast->AsAssignExpression());
      case ASTNodeType::kDeclaration:
        return GenDeclaration(ast->AsDeclaration());

      case ASTNodeType::kDeclarationList:
        return GenDeclarationList(ast->AsDeclarationList());

      case ASTNodeType::kBlockStatement:
        return GenBlockStatement(ast->AsBlockStatement());

      case ASTNodeType::kFunctionStatement:
        return GenFunctionStatement(ast->AsFunctionStatement());

      case ASTNodeType::kReturnStatement:
        return GenReturnStatement(ast->AsReturnStatement());

      case ASTNodeType::kForStatement:
        return GenForStatement(ast->AsForStatement());

      case ASTNodeType::kIfElseStatement:
        return GenIfStatement(ast->AsIfElseStatement());

      default:
        throw std::runtime_error("not yet implmenented");
        return nullptr;
    }
  }


  Ref<Value> GenConstantInt(Ref<IntegralLiteral> literal) {
    return builder_.getConstantInt(literal->value());
  }

  Ref<Value> GenConstantStr(Ref<StringLiteral> literal) {
    return builder_.getConstantStr(literal->string());
  }

  Ref<Value> GenIdentifier(Ref<Identifier> id) {
    auto symbol = manager_->GetSymbol(id->GetName());
    if (address_)
      return symbol->value();
    else {
      return named(builder_.createLoad(symbol->value()));
    }
  }

  Ref<Value> GenFunctionInvoke(Ref<Value> member, Ref<MemberExpression> mem) {
    auto arg_list = mem->member()->AsArgumentList();
    std::vector<Ref<Value>> args;

    AddressEnv<false> env(this);
    for (auto &arg : *arg_list->args()) {
      args.push_back(RunOn(arg));
    }

    auto result = builder_.createInvoke(member->AsFunction(), args);
    return named(result);
  }

  Ref<Value> GenMemberExpression(Ref<MemberExpression> mem) {
    Ref<Value> member;
    {
      AddressEnv<true> env(this);
      member = RunOn(mem->expr());
    }
    switch (mem->kind()) {
      case MemberAccessKind::kCall:
        return named(GenFunctionInvoke(member, mem));

      case MemberAccessKind::kDot: {
        std::string name = mem->member()->AsIdentifier()->GetName();
        auto addr = named(builder_.createGea(member,
          builder_.getConstantStr(name), name));
        if (address_) {
          return addr;
        }

        return named(builder_.createLoad(addr));
      }

      case MemberAccessKind::kIndex:
        return named(builder_.createIdx(member, RunOn(mem->member())));
      default:
        return nullptr;
    }
  }

  Ref<Value> GenPrefixExpression(Ref<PrefixExpression> prefix) {
    AddressEnv<false> env(this);
    auto value = RunOn(prefix->expr());
    decltype(value) addr;
    {
      AddressEnv<true> env(this);
      addr = RunOn(prefix->expr());
    }

    decltype(value) update;
    switch (prefix->op()) {
    case PrefixOperation::kIncrement:
      update = named(builder_.createBinary<OpCode::kAdd>(value, builder_.getConstantInt(1)));
      break;

    case PrefixOperation::kDecrement:
      update = named(builder_.createBinary<OpCode::kSub>(value, builder_.getConstantInt(1)));
      break;

    case PrefixOperation::kAddr:
      return addr;

    case PrefixOperation::kDeref:
      return named(builder_.createLoad(value));

    default:
      assert(false && "Undefined operation");
    }

    auto store = builder_.createStore(addr, update);
    return update;
  }

  Ref<Value> GenPostfixExpression(Ref<PostfixExpression> postfix) {
    AddressEnv<false> env(this);
    auto value = RunOn(postfix->expr());
    decltype(value) addr;
    {
      AddressEnv<true> env(this);
      addr = RunOn(postfix->expr());
    }

    decltype(value) update;

    if (postfix->op() == PostfixOperation::kIncrement)
      update = named(builder_.createBinary<OpCode::kAdd>(value, builder_.getConstantInt(1)));
    else
      update = named(builder_.createBinary<OpCode::kSub>(value, builder_.getConstantInt(1)));
    auto store = builder_.createStore(addr, update);
    return value;
  }

  Ref<Value> GenBinaryExpression(Ref<BinaryExpression> expr) {
    AddressEnv<false> env(this);
    auto lhs = RunOn(expr->lhs());
    auto rhs = RunOn(expr->rhs());

    switch (expr->op()) {
      case BinaryOperation::kAddition:
        return named(builder_.createBinary<OpCode::kAdd>(lhs, rhs));
      case BinaryOperation::kSubtraction:
        return named(builder_.createBinary<OpCode::kSub>(lhs, rhs));
      case BinaryOperation::kMultiplication:
        return named(builder_.createBinary<OpCode::kMul>(lhs, rhs));
      case BinaryOperation::kDivision:
        return named(builder_.createBinary<OpCode::kDiv>(lhs, rhs));
      case BinaryOperation::kMod:
        return named(builder_.createBinary<OpCode::kMod>(lhs, rhs));
      case BinaryOperation::kShiftRight:
        return named(builder_.createBinary<OpCode::kShr>(lhs, rhs));
      case BinaryOperation::kShiftLeft:
        return named(builder_.createBinary<OpCode::kShl>(lhs, rhs));
      case BinaryOperation::kShiftZeroRight:
        return named(builder_.createBinary<OpCode::kSzr>(lhs, rhs));
      case BinaryOperation::kLessThan:
        return named(builder_.createBinary<OpCode::kLt>(lhs, rhs));
      case BinaryOperation::kGreaterThan:
        return named(builder_.createBinary<OpCode::kGt>(lhs, rhs));
      case BinaryOperation::kEqual:
        return named(builder_.createBinary<OpCode::kEq>(lhs, rhs));
      case BinaryOperation::kNotEqual:
        return named(builder_.createBinary<OpCode::kNeq>(lhs, rhs));
      case BinaryOperation::kBitAnd:
        return named(builder_.createBinary<OpCode::kAnd>(lhs, rhs));
      case BinaryOperation::kBitOr:
        return named(builder_.createBinary<OpCode::kOr>(lhs, rhs));

      case BinaryOperation::kGreaterThanEqual:
        return named(builder_.createBinary<OpCode::kGt>(lhs, rhs));

      case BinaryOperation::kLessThanEqual:
        return named(builder_.createBinary<OpCode::kLt>(lhs, rhs));

      default:
        return nullptr;
    }

    return nullptr;
  }

  Ref<Value> GenAssignExpression(Ref<AssignExpression> expr) {
    Ref<Value> value, addr;
    {
      AddressEnv<false> env(this);
      value = RunOn(expr->rhs());
    }

    {
      AddressEnv<true> env(this);
      addr = RunOn(expr->lhs());
    }
    builder_.createStore(addr, value);

    if (!address_)
      return named(builder_.createLoad(addr));
    return nullptr;
  }

  Ref<Value> GenDeclaration(Ref<Declaration> decl) {
    Ref<Value> alloc = builder_.createAlloc(decl->type(), decl->name());
    manager_->PutSymbol(decl->name(), alloc->getType(), alloc);
    AddressEnv<true> env(this);
    if (decl->expr()) {
      AddressEnv<false> env(this);
      builder_.createStore(alloc, RunOn(decl->expr()));
    }
    return alloc;
  }

  Ref<Value> GenDeclarationList(Ref<DeclarationList> list) {
    for (auto &decl : list->exprs()) {
      GenDeclaration(decl);
    }

    return nullptr;
  }

  Ref<Value> GenBlockStatement(Ref<BlockStatement> block) {
    for (auto &expr : *block->statements()) {
      RunOn(expr);
    }

    return nullptr;
  }

  void GenParams(const std::vector<std::string> &params, FunctionType *type) {
    auto i = 0;
    for (auto &param : params) {
      Ref<Value> p(new Parameter(type->getArgumentsTypes()[i], i));
      manager_->PutSymbol(param, type->getArgumentsTypes()[i++], p);
    }
  }

  Ref<Value> GenFunctionStatement(Ref<FunctionStatement> statement) {
    Ref<Function> function(new Function());
    auto name = statement->proto()->GetName();
    function->setName(name);
    auto ftype = statement->proto()->GetType()->AsFunctionType();
    function->setType(ftype);

    manager_->PutSymbol(name, ftype, function);
    mod_->Add(name, function);

    if (statement->IsExtern()) {
      function->setExtern(true);
      return function;
    }
    Ref<BasicBlock> bb(new BasicBlock(function, "entry"));
    function->AddBB(bb);

    // save any function that was previously codegened
    auto old = builder_.GetFunction();
    auto oldbb = builder_.GetBasicBlock();

    builder_.SetFunction(function);
    builder_.SetBasicBlock(bb);

    GenParams(statement->proto()->GetArgs(), ftype);
    RunOn(statement->body());


    // restore the builder
    builder_.SetFunction(old);
    builder_.SetBasicBlock(oldbb);
    return function;
  }

  Ref<Value> GenForStatement(Ref<ForStatement> f) {
    RunOn(f->init());

    Ref<BasicBlock> condition(new BasicBlock(builder_.GetFunction()));
    Ref<BasicBlock> body(new BasicBlock(builder_.GetFunction()));
    Ref<BasicBlock> end(new BasicBlock(builder_.GetFunction()));
    std::string name = namer_.Name(condition);
    condition->setLabel(name);
    name = namer_.Name(body);
    body->setLabel(name);
    name = namer_.Name(end);
    end->setLabel(name);
    builder_.GetFunction()->AddBB(condition);
    builder_.GetFunction()->AddBB(body);
    builder_.GetFunction()->AddBB(end);

    builder_.createJmp(condition);

    // switch to second basic block
    builder_.SetBasicBlock(body);
    RunOn(f->body());
    RunOn(f->update());
    builder_.createJmp(condition);

    builder_.SetBasicBlock(condition);
    auto cond = RunOn(f->condition());
    builder_.createBrk(cond, body);
    builder_.createJmp(end);

    builder_.SetBasicBlock(end);
    return nullptr;
  }

  Ref<Value> GenIfStatement(Ref<IfElseStatement> i) {
    Ref<BasicBlock> body(new BasicBlock(builder_.GetFunction()));
    Ref<BasicBlock> els(new BasicBlock(builder_.GetFunction()));
    Ref<BasicBlock> next(new BasicBlock(builder_.GetFunction()));
    std::string name = namer_.Name(body);
    body->setLabel(name);
    name = namer_.Name(els);
    els->setLabel(name);
    name = namer_.Name(next);
    next->setLabel(name);
    builder_.GetFunction()->AddBB(body);
    builder_.GetFunction()->AddBB(els);
    builder_.GetFunction()->AddBB(next);

    auto cond = RunOn(i->condition());
    builder_.createBrk(cond, body);

    if (i->els())
      builder_.createJmp(els);
    else
      builder_.createJmp(next);
    builder_.SetBasicBlock(body);
    RunOn(i->body());
    builder_.createJmp(next);

    if (i->els()) {
      builder_.SetBasicBlock(els);
      RunOn(i->els());
      builder_.createJmp(next);
    }

    builder_.SetBasicBlock(next);
    return nullptr;
  }

  Ref<Value> GenReturnStatement(Ref<ReturnStatement> r) {
    Ref<Value> val = RunOn(r->expr());
    return builder_.createRet(val);
  }

  Ref<Value> named(Ref<Value> val) {
    auto instr = dynamic_cast<Instruction*>(val.get());
    auto name = namer_.Name(val);
    if (instr)
      instr->SetName(name);
    return val;
  }

private:
  IRBuilder builder_;
  bool address_;
  Ref<SymbolTableManager> manager_;
  TemporaryNameAssigner namer_;
  Ref<Module> mod_;
};

NewBasicBlock::NewBasicBlock(CodeGeneratorImpl *impl, const std::string &name)
    : impl_{ impl }, bb_{ impl->builder_.GetBasicBlock() }
{
  Ref<BasicBlock> bb(new BasicBlock(impl->builder_.GetFunction(), name));
  impl_->builder_.SetBasicBlock(bb);
}

NewBasicBlock::~NewBasicBlock() {
  impl_->builder_.SetBasicBlock(bb_);
}

template <bool load>
AddressEnv<load>::AddressEnv(CodeGeneratorImpl *impl)
    : impl_{ impl }, prev_{ impl->address_ }
{
  impl_->address_ = load;
}

template <bool load>
AddressEnv<load>::~AddressEnv() {
  impl_->address_ = prev_;
}

CodeGenerator::CodeGenerator()
  : impl_{ nullptr }
{ }

CodeGenerator::~CodeGenerator() {
}

Ref<Module> CodeGenerator::RunOn(Ref<Expression> ast) {
  Ref<SymbolTableManager> manager(new SymbolTableManager());
  impl_ = new CodeGeneratorImpl(manager);
  Ref<Module> module(new Module());
  impl_->RunOn(ast, module);
  delete impl_;
  return module;
}

}
