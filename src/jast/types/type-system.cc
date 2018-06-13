#include "jast/types/type-system.h"
#include "jast/types/type.h"

#include "jast/strings.h"
#include "jast/common.h"

#include <map>
#include <list>
#include <assert.h>

using namespace jast;

namespace jast {
std::unordered_map<std::string, Type*> TypeRegistry::named_types_;
}

void TypeRegistry::Register(const std::string &name, Type *type) {
    auto it = named_types_.find(name);
    if (it == named_types_.end()) {
        named_types_.insert(std::make_pair(name, type));
        return;
    }

    Type *already = it->second;
    if (already->IsUnresolvedType()) {
        if (already->AsUnresolvedType()->IsResolved()) {
            throw TypeError(Strings::Format("{} has been already resolved", name));
        }
        already->AsUnresolvedType()->ResolveTo(type);
    } else {
        throw TypeError(Strings::Format("{} has been already declared", name));
    }
}

Type* TypeRegistry::GetNamedType(const std::string &name) {
    auto it = named_types_.find(name);
    if (it == named_types_.end()) {
        return nullptr;
    }

    return it->second;
}

RegisterNamedType::RegisterNamedType(const std::string &name, Type *type) {
    TypeRegistry::Register(name, type);
}

inline void hash_combine(std::size_t& seed) { }

template <typename T, typename... Rest>
inline void hash_combine(std::size_t& seed, const T& v, Rest... rest) {
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
    hash_combine(seed, rest...);
}

#define MAKE_HASHABLE(type, ...) \
    namespace std {\
        template<> struct hash<type> {\
            std::size_t operator()(const type &t) const {\
                std::size_t ret = 0;\
                hash_combine(ret, __VA_ARGS__);\
                return ret;\
            }\
        };\
    }

namespace std {
    template<>
    struct hash<std::pair<const std::string, Type*>> {
        std::size_t operator()(const std::pair<const std::string, Type*> &t) const {
            std::size_t ret = 0;
            hash_combine(ret, t.first, t.second);
            return ret;
        }
    };
}

struct FunctionKey {
    Type *return_type;
    std::vector<Type*> args_types;
};

bool operator==(const FunctionKey &key, const FunctionKey &key1) {
    bool equal = key.return_type == key1.return_type
                    && key.args_types.size() == key1.args_types.size();
    if (!equal)
        return false;

    auto s = key.args_types.size();
    decltype(s) i = decltype(s)(0);
    for (; i < s; i++) {
        if (key.args_types[i] != key1.args_types[i])
            return false;
    }

    return true;
}

inline size_t calculateHashForFunction(const FunctionKey &key) {
    std::size_t seed = 0;
    hash_combine(seed, key.return_type);
    for (const auto &item : key.args_types) {
        hash_combine(seed, item);
    }

    return seed;
}

namespace std {
    template <>
    struct hash<FunctionKey> {
        std::size_t operator()(const FunctionKey &key) const {
            return calculateHashForFunction(key);
        }
    };
}


inline size_t calculateHash(const std::map<std::string, Type*> &map) {
    std::size_t seed = 0;
    for (const auto &item : map) {
        hash_combine(seed, item);
    }

    return seed;
}

namespace std {
    template<>
    struct hash<std::map<std::string, Type*>> {
        std::size_t operator()(const std::map<std::string, Type*> &t) const {
            return calculateHash(t);
        }
    };
}


namespace jast {

Type* TypeSystem::getIntegerType(const std::string &name) {
    if (name.length() > 0) {
        auto t = TypeRegistry::GetNamedType(name);
        if (t == nullptr) {
            TypeRegistry::Register(name, getIntegerType());
        }
    }

    return IntegerType::get();
}

Type* TypeSystem::getStringType(const std::string &name) {
    if (name.length() > 0) {
        auto t = TypeRegistry::GetNamedType(name);
        if (t == nullptr) {
            TypeRegistry::Register(name, getIntegerType());
        }
    }

    return StringType::get();}

Type* TypeSystem::getUndefinedType() {
    return UndefinedType::get();
}

Type* TypeSystem::getArrayType(Type *base_type, bool size_known, size_t size,
        const std::string &name) {
    static std::map<Type*, ArrayType> types;
    auto it = types.find(base_type);
    Type *ret = nullptr;
    if (it == types.end()) {
        auto n = types.insert({base_type, ArrayType(base_type, size_known, size)});
        ret = &(n.first->second);
    } else {
        ret = &(it->second);
    }

    if (name.length() > 0) {
        auto t = TypeRegistry::GetNamedType(name);
        if (t == nullptr) {
            t = ret;
            TypeRegistry::Register(name, t);
        }
        assert(t == ret && "Types are not same");
    }
    return ret;
}

Type* TypeSystem::getPointerType(Type *base_type, const std::string &name) {
    static std::map<Type*, PointerType> types;
    auto it = types.find(base_type);
    Type *ret = nullptr;
    if (it == types.end()) {
        auto n = types.insert({base_type, PointerType(base_type)});
        ret = &(n.first->second);
    } else {
        ret = &(it->second);
    }

    if (name.length() > 0) {
        auto t = TypeRegistry::GetNamedType(name);
        if (t == nullptr) {
            t = ret;
            TypeRegistry::Register(name, t);
        }
        assert(t == ret && "Types are not same");
    }
    return ret;
}

class ObjectTypeManager {
public:
    static std::unordered_map<std::map<std::string, Type*>, ObjectType> types_;
};

std::unordered_map<std::map<std::string, Type*>, ObjectType> ObjectTypeManager::types_;

Type* TypeSystem::getObjectType(const std::map<std::string, Type*> &map,
        const std::string &name) {
    std::unordered_map<std::map<std::string, Type*>, ObjectType> &types = ObjectTypeManager::types_;
    auto it = types.find(map);
    if (it == types.end()) {
        auto n = types.insert(std::make_pair(map, ObjectType(map)));
        if (name.length() > 0) {
            TypeRegistry::Register(name, &n.first->second);
        }
        return &(n.first->second);
    }

    return &it->second;
}

Type* TypeSystem::getUnresolvedType() {
    static std::list<UnresolvedType> types;
    types.push_back(UnresolvedType());
    return &types.back();
}

Type* TypeSystem::getFunctionType(Type *return_type, const std::vector<Type*> &args_types,
    const std::string &name) {
    static std::unordered_map<FunctionKey, FunctionType> types;

    FunctionKey key;
    key.return_type = return_type;
    key.args_types = args_types;
    Type *ret = nullptr;
    auto it = types.find(key);
    if (it == types.end()) {
        auto n = types.insert(std::make_pair(key, FunctionType(return_type, args_types)));
        ret = &(n.first->second);
    } else {
        ret = &(it->second);
    }

    if (name.length() > 0) {
        auto t = TypeRegistry::GetNamedType(name);
        if (t == nullptr) {
            t = ret;
            TypeRegistry::Register(name, t);
        }
        assert(t == ret && "Types are not same");
    }
    return ret;
}

Type* TypeSystem::getNamedType(const std::string &name) {
    auto t = TypeRegistry::GetNamedType(name);
    if (t == nullptr) {
        // save the named type as unresolved type
        t = getUnresolvedType();
        TypeRegistry::Register(name, t);
    }

    return t;
}

void TypeSystem::AliasType(const std::string &name, const std::string &originalTypeName) {
    Type *t = getNamedType(originalTypeName);
    TypeRegistry::Register(name, t);
}

}
