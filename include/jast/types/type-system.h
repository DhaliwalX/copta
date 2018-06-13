#ifndef TYPE_SYSTEM_H_
#define TYPE_SYSTEM_H_

#include <vector>
#include <map>
#include <unordered_map>

namespace jast {

class Type;

/*
 * TypeSystem: this class manages all the types and ensures that every type
 * has only single instance.
 */
class TypeSystem {
public:
    // name is optional for integer, string, array, pointer and function types
    static Type *getIntegerType(const std::string &name = "");
    static Type *getStringType(const std::string &name = "");
    static Type *getUndefinedType();
    static Type *getArrayType(Type *base_type, bool size_known = false,
                      size_t size = 0, const std::string &name = "");
    static Type *getPointerType(Type *base_type, const std::string &name = "");

    // name is required for object type
    static Type *getObjectType(const std::map<std::string, Type*> &map_,
                      const std::string &name);
    static Type *getFunctionType(Type *return_type, const std::vector<Type*> &args_types,
                      const std::string &name = "");

    // special type that always has a unique instance
    static Type *getUnresolvedType();

    static Type *getNamedType(const std::string &name);

    static void AliasType(const std::string &name, const std::string &originalTypeName);
};

// registry of all the named types. XXX: Type without name is not supported
class TypeRegistry {
public:
  static void Register(const std::string &name, Type *type);
  static Type *GetNamedType(const std::string &name);
private:
  static std::unordered_map<std::string, Type*> named_types_;
};

class RegisterNamedType {
public:
  RegisterNamedType(const std::string &name, Type *type);
};

}

#endif
