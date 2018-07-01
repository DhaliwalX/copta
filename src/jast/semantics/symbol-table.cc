#include "jast/semantics/symbol-table.h"
#include "jast/types/type.h"
#include "jast/ir/value.h"
#include <iostream>

namespace jast {

SymbolDefinition::SymbolDefinition(const std::string &name, Type *type, Ref<Value> value)
  : name_{name}, type_{ type}, depth_{0}, last_{nullptr}, next_{nullptr}, value_{value}
{ }

SymbolTable::SymbolTable()
  : entries_(1)
{ }

SymbolTable::~SymbolTable() {
  while (currentDepth_ >= 0) {
    CloseScope();
  }
}

void SymbolDefinition::dump() const {
  std::cout << "@" << depth_;
  std::cout << "(";
  type_->dump();
  std::cout << ")";
  if (next_) {
    std::cout << " -> ";
    next_->dump();
  }
}

SymbolDefinition *SymbolTable::GetSymbol(const std::string &name) {
  auto it = table_.find(name);
  if (it == table_.end())
    return nullptr;

  return (it->second);
}

SymbolDefinition *SymbolTable::GetSymbolInCurrentScope(const std::string &name) {
  auto sym = GetSymbol(name);
  if (sym == nullptr)
    return nullptr;
  if (sym->depth_ == currentDepth_)
    return sym;
  return nullptr;
}

SymbolDefinition *SymbolTable::PutSymbol(const std::string &name, Type *type) {
  return PutSymbol(name, type, nullptr);
}

SymbolDefinition *SymbolTable::PutSymbol(const std::string &name, Type *type,
    Ref<Value> value) {
  auto sym = GetSymbol(name);
  if (sym != nullptr && sym->depth_ == currentDepth_) {
    return nullptr;
  }
  auto def = new SymbolDefinition(name, type, value);
  def->depth_ = currentDepth_;

  if (sym) {
    sym->last_ = def;
    def->next_ = sym;
  }

  table_[name] = def;
  entries_[currentDepth_].push_back(def);

  return def;
}

void SymbolTable::OpenScope() {
  currentDepth_++;
  entries_.push_back(SymbolEntryList{});
}

void SymbolTable::CloseScope() {
  for (auto it = entries_[currentDepth_].begin(); it != entries_[currentDepth_].end(); ++it) {
    auto entry = *it;
    if (entry->next_) {
      entry->next_->last_ = nullptr;
      table_[entry->name_] = entry->next_;
    } else {
      table_.erase(table_.find(entry->name_));
    }

    delete entry;
  }
  entries_[currentDepth_].clear();
  currentDepth_--;
}

void SymbolTable::dump() const {
  std::cout << "{{\n";
  for (const auto &entry : table_) {
    std::cout << "  " << entry.first;
    entry.second->dump();
    std::cout << std::endl;
  }

  std::cout << "}}" << std::endl;
}

}
