#ifndef TYPE_H_
#define TYPE_H_

#include <map>
#include <vector>
#include <string>

namespace jast {

#define TYPE_LIST(E) \
    E(Integer) \
    E(String) \
\
    /* Array type */\
    E(Array) \
\
    /* Object types */ \
    E(Object) \
\
    /* Function type */ \
    E(Function) \
\
    /* Pointer type */ \
    E(Pointer) \
    E(Unresolved) \
    E(Undefined)

enum class TypeID : unsigned {
#ifndef TYPE_ID
#define TYPE_ID(T) T,
TYPE_LIST(TYPE_ID)
#undef TYPE_ID
#endif
};

template <typename T>
using Vector = std::vector<T>;
using String = std::string;

static inline std::string type_id_name(TypeID id) {
    switch (id) {
#define CASE(t) case TypeID::t: return #t;
TYPE_LIST(CASE)
#undef CASE
        default:
            return "undef";
    }
}

#define FORWARD_DECLARE(T) class T##Type;
TYPE_LIST(FORWARD_DECLARE)
#undef FORWARD_DECLARE

class ArrayType;

// Type is the base class for all the types present in a program
class Type {
protected:
    Type(TypeID type_id)
        : type_id_{ type_id }
    { }

    virtual ~Type() = default;

public:
    virtual const String &getName() const = 0;

    TypeID getTypeID() const {
        return type_id_;
    }
    virtual bool Equals(Type *that) const {
        return type_id_ == that->type_id_;
    }

    virtual bool IsIntegerType() const {
        return type_id_ == TypeID::Integer;
    }

    virtual bool IsStringType() const {
        return type_id_ == TypeID::String;
    }
    virtual bool IsArrayType() const {
        return type_id_ == TypeID::Array;
    }

    virtual bool IsObjectType() const {
        return type_id_ == TypeID::Object;
    }

    virtual bool IsFunctionType() const {
        return type_id_ == TypeID::Function;
    }

    virtual bool IsPointerType() const {
        return type_id_ == TypeID::Pointer;
    }

    virtual bool IsUnresolvedType() const {
        return type_id_ == TypeID::Unresolved;
    }

    virtual bool IsUndefinedType() const {
        return type_id_ == TypeID::Undefined;
    }
    virtual void dump() const = 0;

#define AS_TYPE(T) virtual T##Type* As##T##Type() { return nullptr; }
    TYPE_LIST(AS_TYPE)
#undef AS_TYPE

protected:
    TypeID type_id_;
};

// NamedType provides name to the type
class NamedType : public Type {
protected:
    explicit NamedType(TypeID type_id, const String &name = "")
        : Type(type_id), name_{name}
    { }

public:
    const String &getName() const override {
        return name_;
    }

    bool Equals(Type *that) const override {
        return Type::Equals(that) && name_ == dynamic_cast<NamedType*>(that)->name_;
    }

    void dump() const override;
protected:
    String name_;
};

// IntegerType represents all the integer classes
class IntegerType : public NamedType {
    friend class TypeSystem;
protected:
    IntegerType(TypeID type_id)
        : NamedType(type_id, type_id_name(type_id))
    { }

public:
    IntegerType* AsIntegerType() override {
        return this;
    }

    void dump() const override;
public:
    static IntegerType* get() {
        static IntegerType type(TypeID::Integer);
        return &type;
    }
};

class StringType : public NamedType {
    friend class TypeSystem;
protected:
    StringType(TypeID type_id)
        : NamedType(type_id, type_id_name(type_id))
    { }

public:
    static StringType* get() {
        static StringType type(TypeID::String);
        return &type;
    }

    StringType *AsStringType() override {
        return this;
    }
    void dump() const override;
};

class ArrayType : public NamedType {
    friend class TypeSystem;
protected:
    ArrayType(Type *elementType, bool size_known = false,
        std::size_t size = 0, const String &name = "")
        : NamedType(TypeID::Array, name),
          element_type_(elementType),
          size_known_{size_known},
          size_{size}
    { }

public:
    Type *getBaseElementType() {
        return element_type_;
    }

    ArrayType* AsArrayType() override {
        return this;
    }

    bool Equals(Type *that) const override {
        return NamedType::Equals(that) && that->AsArrayType()->element_type_->Equals(element_type_);
    }

    void dump() const override;
protected:
    Type *element_type_;
    bool size_known_;
    std::size_t size_;
};

class PointerType : public NamedType {
    friend class TypeSystem;
protected:
    PointerType(Type *base, const String &name = "")
        : NamedType(TypeID::Pointer, name), base_type_{ base }
    { }
public:
    Type *getBaseElementType() {
        return base_type_;
    }

    PointerType* AsPointerType() override {
        return this;
    }

    bool Equals(Type *that) const override {
        return NamedType::Equals(that) && that->AsPointerType()->base_type_->Equals(base_type_);
    }

    void dump() const override;

    static PointerType *get(Type *base);
private:
    Type *base_type_;
};

class ObjectType : public NamedType {
    friend class TypeSystem;
protected:
    ObjectType(std::map<std::string, Type*> s, const String &name = "")
        : NamedType(TypeID::Object, name), struct_{ s }
    { }

public:
    using iterator = std::map<std::string, Type*>::iterator;
    using const_iterator = std::map<std::string, Type*>::const_iterator;

    iterator begin() {
        return struct_.begin();
    }

    Type *getMember(const std::string &name) {
        auto it = struct_.find(name);
        if (it == struct_.end()) {
            return nullptr;
        }

        return it->second;
    }
    iterator end() {
        return struct_.end();
    }

    const_iterator begin() const {
        return struct_.begin();
    }

    const_iterator end() const {
        return struct_.end();
    }

    ObjectType* AsObjectType() override {
        return this;
    }

    bool Equals(Type *that) const override {
        if (!NamedType::Equals(that)) {
            return false;
        }

        bool equal = false;

        auto casted = that->AsObjectType();
        if (struct_.size() != casted->struct_.size()) {
            return false;
        }

        for (const auto &t : struct_) {
            auto it = casted->struct_.find(t.first);
            if (it == casted->struct_.end())
                return false;

            equal = it->second->Equals(t.second);
            if (!equal)
                return false;
        }

        return equal;
    }

    void dump() const override;

    static ObjectType *get(std::map<std::string, Type*> s);
private:
    std::map<std::string, Type*> struct_;
};

class FunctionType : public NamedType {
    friend class TypeSystem;
protected:
    FunctionType(Type *return_type,
                    Vector<Type*> argument_type,
                    const String &name = "")
        : NamedType(TypeID::Function, name), return_type_{ return_type },
            args_types_{ argument_type }
    { }

public:
    Type *getReturnType() {
        return return_type_;
    }

    Vector<Type*> getArgumentsTypes() {
        return args_types_;
    }

    FunctionType* AsFunctionType() override {
        return this;
    }

    bool Equals(Type *that) const override {
        if (!NamedType::Equals(that)) {
            return false;
        }
        auto casted = that->AsFunctionType();

        if (!return_type_->Equals(casted->return_type_)) {
            return false;
        }

        bool equal = false;

        if (args_types_.size() != casted->args_types_.size()) {
            return false;
        }

        for (auto i = size_t(0); i < args_types_.size(); i++) {
            if (!args_types_[i]->Equals(casted->args_types_[i])) {
                equal = false;
                break;
            }
        }

        return equal;
    }

    static FunctionType *get(Type *return_type, Vector<Type*> args_types);

    void dump() const override;
private:
    Type *return_type_;
    Vector<Type*> args_types_;
};

class UndefinedType : public NamedType {
    friend class TypeSystem;
protected:
    UndefinedType()
        : NamedType(TypeID::Undefined, type_id_name(TypeID::Undefined))
    {}
public:
    static UndefinedType* get() {
        static UndefinedType type;
        return &type;
    }
};

class UnresolvedType : public NamedType {
    friend class TypeSystem;
protected:
    UnresolvedType()
        : NamedType(TypeID::Unresolved, type_id_name(TypeID::Unresolved)),
        placeholder_{nullptr}, resolved_{false}
    {}

public:
    void ResolveTo(Type *t) {
        if (t->IsUnresolvedType()) {
            ResolveTo(t->AsUnresolvedType()->ResolvedType());
            return;
        }
        resolved_ = true;
        placeholder_ = t;
    }

    bool IsResolved() const {
        return resolved_;
    }

    Type *ResolvedType() {
        return placeholder_;
    }

#define IS_TYPE(T)\
    bool Is##T##Type() const override {\
        if (TypeID::Unresolved == TypeID::T) { \
            return true;\
        }\
        if (IsResolved()) {\
            return placeholder_->Is##T##Type();\
        }\
\
        return false;\
    }

TYPE_LIST(IS_TYPE)
#undef IS_TYPE

#define AS_TYPE(T)\
    T##Type *As##T##Type() override {\
        if (TypeID::Unresolved == TypeID::T) {\
            return dynamic_cast<T##Type*>(this);\
        }\
        if (IsResolved()) {\
            return placeholder_->As##T##Type();\
        }\
        return nullptr; \
    }

TYPE_LIST(AS_TYPE)
#undef AS_TYPE

    bool Equals(Type *that) const override {
        if (!IsResolved()) {
            return false;
        }
        if (that->IsUnresolvedType()) {
            auto *t = dynamic_cast<UnresolvedType*>(that);
            if (!t->IsResolved()) {
                return false;
            }

            return t->ResolvedType() == placeholder_;
        }

        return that == placeholder_;
    }

    void dump() const override;
private:
    Type *placeholder_;
    bool resolved_;
};

}

#endif
