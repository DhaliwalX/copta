#include "jast/types/type-system.h"
#include "jast/types/type.h"

#include <unordered_map>
#include <map>
#include <list>

using namespace jast;

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

Type* TypeSystem::getIntegerType() {
    return IntegerType::get();
}

Type* TypeSystem::getStringType() {
    return StringType::get();
}

Type* TypeSystem::getUndefinedType() {
    return UndefinedType::get();
}

Type* TypeSystem::getArrayType(Type *base_type, bool size_known, size_t size) {
    static std::map<Type*, ArrayType> types;
    auto it = types.find(base_type);
    if (it == types.end()) {
        auto n = types.insert({base_type, ArrayType::get(base_type, size_known, size)});
        return &(n.first->second);
    }

    return &(it->second);
}

Type* TypeSystem::getPointerType(Type *base_type) {
    static std::map<Type*, PointerType> types;
    auto it = types.find(base_type);
    if (it == types.end()) {
        auto n = types.insert({ base_type, PointerType::get(base_type) });
        return &(n.first->second);
    }

    return &it->second;
}

Type* TypeSystem::getObjectType(const std::map<std::string, Type*> &map) {
    static std::unordered_map<std::map<std::string, Type*>, ObjectType> types;
    auto it = types.find(map);
    if (it == types.end()) {
        auto n = types.insert(std::make_pair(map, ObjectType(map)));
        return &(n.first->second);
    }

    return &it->second;
}

Type* TypeSystem::getUnresolvedType() {
    static std::list<UnresolvedType> types;
    types.push_back(UnresolvedType());
    return &types.back();
}

Type* TypeSystem::getFunctionType(Type *return_type, const std::vector<Type*> &args_types) {
    static std::unordered_map<FunctionKey, FunctionType> types;

    FunctionKey key;
    key.return_type = return_type;
    key.args_types = args_types;
    auto it = types.find(key);
    if (it == types.end()) {
        auto n = types.insert(std::make_pair(key, FunctionType(return_type, args_types)));
        return &(n.first->second);
    }

    return &it->second;
}

}
