#ifndef LIST_H_
#define LIST_H_

#include <type_traits>

namespace jast {

template <typename ListType>
class ListIterator {
public:
  enum Tag {
    kBegin,
    kEnd
  };
  using DataType = ListType;
  using reference = ListType&;

  explicit ListIterator(ListType *element, Tag tag)
    : start_(tag == kBegin ? element : nullptr),
      element_(tag == kEnd ? nullptr : (element))
  { }

  ListIterator(ListType *start, ListType *point)
    : start_(start), element_(point)
  { }

  DataType *operator->() {
    return element_;
  }

  reference operator*() {
    return *element_;
  }

  ListIterator<ListType> &operator++() {
    element_ = element_->Next;
    return *this;
  }

  ListIterator<ListType> operator++(int) {
    ListIterator ret(element_);
    element_ = element_->Next;
    return ret;
  }

  ListIterator<ListType> &operator--() {
    element_ = element_->Previous;
    return *this;
  }

  ListIterator<ListType> operator--(int) {
    ListIterator ret(element_);
    element_ = element_->Previous;
    return ret;
  }

  operator DataType*() {
    return element_;
  }

  bool operator==(const ListIterator<ListType> &rhs) const {
    if (rhs.start_ == nullptr) {
      return element_->Next == start_;
    }

    return rhs.start_ == start_ && rhs.element_ == element_;
  }
  DataType *start_;
  DataType *element_;
};

/**
 * Embeddable doubly linked list
 * List is not the owner of the members. It requires a default constructor
 */
template <typename T>
class List {
  friend class ListIterator<List<T>>;
public:
  using DataType = T;
  using reference = T&;
  using iterator = ListIterator<List<T>>;

protected:
  List() {
    Next = nullptr;
    Previous = nullptr;
    Last = (T*)this;
  }

public:

  /**
   * Do nothing destructor
   */
  virtual ~List() {}

  /**
   * append to the list
   */
  void Append(T *element) {
    element->Previous = Last->Previous;
    if (Last->Previous) {
      Last->Previous->Next = element;
    } else {
      Next = element;
    }
    Last = element;
  }

  /**
   * removes the last element from the list
   */
  T *RemoveLast() {
    T *ret = Last;

    if (Last->Previous) {
      return nullptr;
    }
    Last = Last->Previous;
    Last->Next = nullptr;
    return ret;
  }

  /**
   * Removes itself and returns pointer to the next element in the list
   */
  T *RemoveSelf() {
    T *ret = Next;
    if (Next == Previous) {
      // single element, don't have to do anything
      return this;
    }

    Previous->Next = Next;
    Next->Previous = Previous;
    Previous = nullptr;
    Next = nullptr;
    return Next;
  }

  /**
   * standard container operations
   */
  iterator begin() {
    return iterator((this));
  }

  iterator end() {
    return iterator(Last);
  }

  T* AsType() {
    return dynamic_cast<T*>(this);
  }

  T* next() {
    return (Next);
  }

  const T* next() const {
    return (Next);
  }

  T* prev() {
    return Previous;
  }

  const T* prev() const {
    return Previous;
  }

  /**
   * inserts element after `point` and returns iterator to the inserted
   * element.
   */
  iterator Insert(iterator point, T *element) {
    assert(point->Next != nullptr);
    element->Next = point->Next;
    element->Previous = &(*point);
    point->Next = element;
    return ++point;
  }

  /**
   * Removes the element pointed by `point`
   */
  iterator Remove(iterator point) {
    return iterator(this, point->RemoveSelf());
  }

  T *Next;
  T *Previous;

private:
  T *Last;
};

}

#endif
