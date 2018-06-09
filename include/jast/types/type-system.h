#ifndef TYPE_SYSTEM_H_
#define TYPE_SYSTEM_H_

#include <vector>
#include <map>

namespace jast {

class Type;

/*
 * TypeSystem: this class manages all the types and ensures that every type
 * has only single instance.
 */
class TypeSystem {
public:
    static Type *getIntegerType();
    static Type *getStringType();
    static Type *getUndefinedType();
    static Type *getArrayType(Type *base_type, bool size_known = false, size_t size = 0);
    static Type *getPointerType(Type *base_type);
    static Type *getObjectType(const std::map<std::string, Type*> &map_);
    static Type *getFunctionType(Type *return_type, const std::vector<Type*> &args_types);

    // special type that always has a unique instance
    static Type *getUnresolvedType();

};

}

#endif
