#include <jast/list.h>

#include <jast/handle.h>
#include <gtest/gtest.h>

#include <iostream>
#include <cassert>

using namespace jast;

namespace {

class Dummy : public RefCountObject {
public:
    Dummy(int i)
        : i (i)
    { }

    void dump() {
        std::cout << "Refs(" << GetNumReferences() << ") " << i << std::endl;
    }
private:
    int i;
};

TEST(HandleTests, Refs) {
    Ref<Dummy> one(new Dummy(1));
    Ref<Dummy> two(one);
    one->dump();
    two->dump();
    {
        Ref<Dummy> three(one);
        one->dump();
    }

    one->dump();
}

class B : public RefCountObject {
public:
    ~B() {
        std::cout << "Destructor called" << std::endl;
    }
};

class A : public B {
public:
    ~A() {
        std::cout << "A's destructor called" << std::endl;
    }
};

TEST(HandleTests, Inheritance) {
    Ref<A> b(MakeHandle<A>());
    {
    Ref<B> a = b;
    std::cout << a->GetNumReferences() << std::endl;
    std::cout << b->GetNumReferences() << std::endl;
    }
    std::cout << b->GetNumReferences() << std::endl;
}

}

