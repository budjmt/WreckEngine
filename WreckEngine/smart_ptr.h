#pragma once

#include <memory>
#include <vector>

using std::make_unique;
using std::make_shared;

// Works the same as unique_ptr, but performs deep-copies instead of ownership transfer; type must be copyable
// it's ok to inherit in this context because the destructor behaves identically and there are no new members
template<class T, class Deleter = std::default_delete<T>>
struct alloc_ptr : public std::unique_ptr<T, Deleter> {
    using unique_ptr::unique_ptr;
    alloc_ptr() = default;
    alloc_ptr(alloc_ptr&&) = default;
    alloc_ptr& operator=(alloc_ptr&&) = default;
    alloc_ptr(const alloc_ptr& o) { reset(o ? new T(*o) : nullptr); }
    alloc_ptr& operator=(const alloc_ptr& o) { alloc_ptr<T> tmp(o); return *this = std::move(tmp); }
};

// Wrapper for thread-safe access to some resource; the wrapper must exist to access the object, and thus the lock persists
// Recommended usage guidelines for best performance:
//   1. Don't recreate it unnecessarily, just save a copy locally (removes unnecessary locks/unlocks)
//   2. Don't keep it alive longer than necessary; it retains the lock for it's entire lifetime (reduces retention)
// These requirements can often be met with an additional level of scope around where it is used
template<typename T, class Lock>
struct safe_ptr {
    using mutex_t = std::remove_pointer_t<std::result_of_t<decltype(&Lock::mutex)(Lock)>>;

    safe_ptr(T* ptr, mutex_t& mut) : value(ptr), lock(mut) {}
    template<typename = std::enable_if_t<std::is_copy_constructible<T>::value>>
    auto operator*() { return *value; }
    auto operator->() { return value; }

    safe_ptr(safe_ptr&&) = default;
    safe_ptr& operator=(safe_ptr&&) = default;
private:
    T* value;
    Lock lock;
};

template<class T, class... Args>
alloc_ptr<T> make_alloc(Args&&... args) { return alloc_ptr<T>{make_unique<T>(args)}; }

template<class T>
alloc_ptr<T> make_alloc(size_t size) { return alloc_ptr<T>{make_unique(size)}; }

template<class T, class Deleter = std::default_delete<T>>
using unique = std::unique_ptr<T, Deleter>;

template<class T> using shared = std::shared_ptr<T>;
template<class T> using weak = std::weak_ptr<T>;

template<class T, class Deleter = std::default_delete<T>> 
using alloc = alloc_ptr<T, Deleter>;

template<class T, class Lock> using safe = safe_ptr<T, Lock>;

/*--------------------------------------------------------------------------------------------------
  - Reserved on the heap and can hold anything, with automatic memory management

  - Intended to function as a simple, constant-size stack, referred to here as "the structure"
    - The structure itself provides no semantic information about the data it manages
    - Any code that accesses this data should "know" at the time of access what is contained
    - All stored "members" are values (references will just get copied)
      - This means pointers are the only way to pass by reference

  - In most use cases, this should function analogously to a lambda capture by value, but it will ALWAYS be reserved on the heap
    - While analogous, performance will be significantly worse
    - Only use this when requirements dictate "capture" without lambdas at the caller

  - Another valid analogue would be a type-erased std::tuple
    - Similarly, do not expect comparable performance
    - That being said, this is still pretty fast
--------------------------------------------------------------------------------------------------*/
struct void_array {
    const size_t capacity;

    explicit void_array(size_t _capacity) : capacity(_capacity), data(make_unique<shared<void>[]>(_capacity)) {}

    // peeks at/gets the data [index] elements into the structure and puts it on the stack as a variable of type T
    template<typename T> inline T peek(const size_t index) const { return *(T*)data[index].get(); }

    // pushes a copy of [d] from the stack into the structure on the heap; assumes T is copy constructible
    template<typename T> inline void push(const T& d) { data[size++] = make_shared<T>(d); }

    // populates with the variables passed as parameters; types are deduced
    template<typename T, typename... Args> void construct(T t, Args&&... args) { push(t); construct(std::forward<Args>(args)...); }
    void construct() {}

    // extracts the contents of the structure in the form of a tuple on the stack, according to the explicit types passed
    template<typename T1, typename T2, typename... Args>
    auto extract() const { return std::tuple_cat(peekTuple<T1>(capacity - sizeof...(Args)-2), extract<T2, Args...>()); }
    // extracts the contents of the structure in the form of a tuple on the stack, according to the explicit types passed
    template<typename T> auto extract() const { return peekTuple<T>(capacity - 1); }

private:
    unique<shared<void>[]> data;
    size_t size = 0;
    template<typename T> inline auto peekTuple(const size_t index) const { return std::tuple<T>(peek<T>(index)); }
};

// simple wrapper for no-copy types
template<typename T>
struct copy_wrap {
    copy_wrap() = default; // deleted because user defined constructor (copy)
    copy_wrap(copy_wrap&&) = default; // deleted because any of the other 4 are defined
    copy_wrap& operator=(copy_wrap&&) = default;
    copy_wrap& operator=(copy_wrap&) = default;
    copy_wrap(const copy_wrap& other) {};
    T object;
};