#pragma once

#include <memory>

using std::make_unique;
using std::make_shared;

// works the same as unique_ptr, but performs deep-copies instead of ownership transfer
template<class T, class Deleter = std::default_delete<T>>
struct alloc_ptr : public std::unique_ptr<T, Deleter> {
	using unique_ptr::unique_ptr;

	alloc_ptr(const alloc_ptr& o) : unique_ptr<T, Deleter>() { reset(o ? new T(*o) : nullptr); }
	alloc_ptr& operator=(const alloc_ptr& o) { reset(o ? new T(*o) : nullptr); return *this; }
};

template<class T, class... Args>
alloc_ptr<T> make_alloc(Args&&... args) { return alloc_ptr<T>(make_unique<T>(args)); }

template<class T>
alloc_ptr<T> make_alloc(size_t size) { return alloc_ptr<T>(make_unique(size)); }

template<class T, class Deleter = std::default_delete<T>>
using unique = std::unique_ptr<T, Deleter>;

template<class T> using shared = std::shared_ptr<T>;
template<class T> using weak = std::weak_ptr<T>;

template<class T> using alloc = alloc_ptr<T>;

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

	void_array(const size_t _capacity) : capacity(_capacity), data(make_unique<shared<void>[]>(_capacity)) {}

	// peeks at/gets the data [index] elements into the structure and puts it on the stack as a variable of type T
	template<typename T> inline T peek(const size_t index) const { return *(T*)data[index].get(); }

	// pushes a copy of [d] from the stack into the structure on the heap; assumes T is copy constructible
	template<typename T> inline void push(const T& d) { data[size++] = make_shared<T>(d); }

	// populates with the variables passed as parameters; types are deduced
	template<typename T, typename... Args> void construct(T t, Args&&... args) { push(t); construct(std::forward<Args>(args)...); }
	void construct() {}

	// extracts the contents of the structure in the form of a tuple on the stack, according to the explicit types passed
	template<typename T1, typename T2, typename... Args>
	auto extract() const { return std::tuple_cat(peekTuple<T1>(capacity - sizeof...(Args) - 2), extract<T2, Args...>()); }
	// extracts the contents of the structure in the form of a tuple on the stack, according to the explicit types passed
	template<typename T> auto extract() const { return peekTuple<T>(capacity - 1); }

private:
	unique<shared<void>[]> data;
	size_t size = 0;
	template<typename T> inline auto peekTuple(const size_t index) const { return std::tuple<T>(peek<T>(index)); }
};