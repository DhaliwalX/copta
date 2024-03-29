#ifndef HANDLE_H_
#define HANDLE_H_


#include <memory>
#include <type_traits>

#include "jast/macros.h"

namespace jast {

class RefCountObject {
public:
  explicit RefCountObject ()
  : reference_count_(0)
  { }

  DISABLE_COPY(RefCountObject);

  virtual ~RefCountObject() = default;

  inline int decrement() {
    return --reference_count_;
  }

  inline int increment() {
    return ++reference_count_;
  }

  inline int GetNumReferences() const {
    return reference_count_;
  }

private:
  int reference_count_;
};

// class T should be a subtype of ReferenceCount class.
template<class T>
class Ref {
  // using Enable = typename std::enable_if<std::is_base_of<RefCountObject, T>::value>::type;
public:
  Ref(T *ptr = nullptr) : ptr_(ptr) {
    if (ptr_ == nullptr)
      return;
    ptr_->increment(); // defined in ReferenceCount
  }

  // Copy Constructor
  Ref(const Ref &ref)
  : ptr_{ ref.ptr_ } {
    if (ptr_ != nullptr)
      ptr_->increment(); // defined in ReferenceCount
  }

  // Ref(Ref<T> &&ref)
  //   : ptr_{ref.ptr_}
  // {
  //   ref.ptr_ = nullptr;
  // }

  // Destructor
  ~Ref() {
    clear();
  }

  // Ref<T> &operator=(Ref<T> &&ref) {
  //   clear();
  //   ptr_ = ref.ptr_;
  //   ref.ptr_ = nullptr;
  //   return *this;
  // }

  Ref<T> &operator=(const Ref<T> &ref) {
    clear();
    ptr_ = ref.ptr_;
    if (ptr_ != nullptr) {
      ptr_->increment();
    }
    return *this;
  }

  // Ref<T> &operator=(T *ptr) {
  //   clear();
  //   if (ptr != nullptr) {
  //     ptr_ = ptr;
  //   }

  //   if (ptr_ != nullptr) {
  //     ptr_->increment();
  //   }
  // }

  inline void clear() {
    if (ptr_ != nullptr) {
      ptr_->decrement();
      if (ptr_->GetNumReferences() <= 0) {
        delete ptr_;
      }
      ptr_ = nullptr;
    }
  }

  inline T& operator*() const noexcept { return *ptr_; }

  inline T* operator->() const noexcept { return ptr_; }

  // Check whether it holds an object.
  inline explicit operator bool() const noexcept {
    return ptr_ != nullptr;
  }

  inline bool operator<(const Ref<T> &ref) const noexcept {
    return GetPtr() < ref.GetPtr();
  }

  inline bool operator<=(const Ref<T> &ref) const noexcept {
    return GetPtr() <= ref.GetPtr();
  }

  inline bool operator==(const Ref<T> &ref) const {
    return ptr_ == ref.GetPtr();
  }

  // preferably use this function over T *get()
  inline T *GetPtr() const {
    return ptr_;
  }

  // redundant func: mimics the library function for std::unique_ptr
  inline T *get() {
    return ptr_;
  }

  inline const T *get() const {
    return ptr_;
  }

  template <class B>
  inline operator Ref<B>() {
      // typename std::enable_if<std::is_base_of<B, T>::value>::type;
      return Ref<B>(dynamic_cast<B*>(ptr_));
    }

private:
  T *ptr_;
};

template <class B, class C>
inline Ref<B> cast(Ref<C> r) {
  return Ref<B>(dynamic_cast<B*>(r.get()));
}

template <typename T>
using Handle = Ref<T>;

template <typename T, typename... Args>
inline Handle<T> MakeHandle(Args&&... args) {
    return Handle<T>(new T(std::forward<Args>(args)...));
}

}

#endif
