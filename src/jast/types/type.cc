#include "jast/types/type.h"
#include "jast/types/type-system.h"
#include "jast/strings.h"
#include <iostream>

namespace jast {

void NamedType::dump() const {
    std::cout << name_;
}

void IntegerType::dump() const {
    NamedType::dump();
}

void StringType::dump() const {
    NamedType::dump();
}

void ArrayType::dump() const {
    if (size_known_) {
        std::cout << "[" << size_ << "]";
    } else {
        std::cout << "[]";
    }

    element_type_->dump();
}

void ObjectType::dump() const {
    std::cout << "struct{";
    for (auto &entry : struct_) {
        std::cout << entry.first << ":";
        entry.second->dump();
        std::cout << ",";
    }

    std::cout << "}";
}

void PointerType::dump() const {
    std::cout << "*";
    base_type_->dump();
}

void FunctionType::dump() const {
    std::cout << "fun(";
    for (auto &arg : args_types_) {
        arg->dump();
        std::cout << ", ";
    }
    std::cout << "):";
    return_type_->dump();
}

void UnresolvedType::dump() const {
    if (IsResolved()) {
        std::cout << "$Resolved<";
        placeholder_->dump();
        std::cout << ">";
        return;
    }
    std::cout << "Unresolved";
}

// static get methods
PointerType *PointerType::get(Type *base) {
    return dynamic_cast<PointerType*>(TypeSystem::getPointerType(base));
}

ObjectType *ObjectType::get(std::map<std::string, Type*> s) {
    return dynamic_cast<ObjectType*>(TypeSystem::getObjectType(s));
}

FunctionType *FunctionType::get(Type *return_type, Vector<Type*> args_types) {
    return dynamic_cast<FunctionType*>(TypeSystem::getFunctionType(return_type, args_types));
}



}