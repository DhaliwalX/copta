#ifndef SEMANTICS_SYMBOL_TABLE_H_
#define SEMANTICS_SYMBOL_TABLE_H_

#include "jast/types/type-system.h"
#include "jast/macros.h"

#include <list>

namespace jast {

class SymbolDefinition {
  friend class SymbolTable;
public:
  SymbolDefinition(const std::string &name, Type *type)
    : name_{ name }, type_{ type}, depth_{ 0 }, last_{ nullptr }, next_{ nullptr }
  { }

  std::string &name() {
    return name_;
  }

  Type *type() {
    return type_;
  }

  int depth() {
    return depth_;
  }

  void dump() const;
private:
  std::string name_;
  Type *type_;
  int depth_;
  SymbolDefinition *last_;
  SymbolDefinition *next_;
};

class SymbolTable {
public:
  SymbolTable();
  ~SymbolTable();
  DISABLE_COPY(SymbolTable);

  SymbolDefinition *GetSymbol(const std::string &name);
  SymbolDefinition *GetSymbolInCurrentScope(const std::string &name);

  SymbolDefinition *PutSymbol(const std::string &name, Type *type);

  void OpenScope();
  void CloseScope();

  void dump() const;
private:
  std::map<std::string, SymbolDefinition*> table_;
  int currentDepth_ = 0;
  using SymbolTableT = std::map<std::string, SymbolDefinition*>;
  using SymbolEntryList = std::vector<SymbolDefinition*>;
  std::vector<SymbolEntryList> entries_;
};

}

#endif
