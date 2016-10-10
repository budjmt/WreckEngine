#pragma once

#include <memory>

using std::make_unique;
using std::make_shared;

// works the same as unique_ptr, but performs a deep-copy on copy instead of transferring ownership
template<class T, class Deleter = std::default_delete<T>>
struct alloc_ptr : public std::unique_ptr<T, Deleter> {
	using unique_ptr::unique_ptr;

	alloc_ptr(const alloc_ptr& o) : unique_ptr<T, Deleter>() { reset(o ? o->clone() : nullptr); }
	alloc_ptr& operator=(const alloc_ptr& o) { reset(o ? o->clone() : nullptr); return *this; }
};

template<class T, class... Args>
//alloc_ptr<T> make_alloc(Args&&... args) { return alloc_ptr<T>(new T(std::forward<Args>(args)...)); }
alloc_ptr<T> make_alloc(Args&&... args) { return alloc_ptr<T>(make_unique<T>(args)); }

template<class T>
//alloc_ptr<T> make_alloc(size_t size) { return alloc_ptr<T>(new typename std::remove_extent<T>::type[size]()); }
alloc_ptr<T> make_alloc(size_t size) { return alloc_ptr<T>(make_unique(size)); }

template<class T, class Deleter = std::default_delete<T>>
using unique = std::unique_ptr<T, Deleter>;

template<class T> using shared = std::shared_ptr<T>;
template<class T> using weak = std::weak_ptr<T>;

template<class T> using alloc = alloc_ptr<T>;

// this is reserved on the heap and can hold anything, while cleaning up after itself as needed
// intended to function as a basic, constant-size stack, internally referred to as "the structure"
// the structure itself provides no semantic information about the data it manages; 
// any code that accesses this data should "know" at the time of access as to what is contained
struct void_array {
	const size_t members;

	void_array(const size_t _members) : members(_members), data(make_unique<shared<void>[]>(_members)) {}

	// peeks at/gets the data [index] elements into the structure and puts it on the stack as a variable of type T
	template<typename T> inline T peek(const size_t index) { return *reinterpret_cast<T*>(data[index].get()); }

	// pushes a copy of [d] from the stack into the structure on the heap; assumes T is copy constructible
	template<typename T> inline void push(const T& d) { data[size++] = make_shared<T>(d); }

	// populates with the variables passed as parameters; types are deduced
	template<typename T, typename... Args> inline void construct(T t, Args&&... args) { push(t); construct(std::forward<Args>(args)...); }
	inline void construct() {}

	// extracts the contents of the structure in the form of a tuple on the stack, according to the explicit types passed
	template<typename T1, typename T2, typename... Args>
	std::tuple<T1, T2, Args...> extract() { return std::tuple_cat(peekTuple<T1>(members - sizeof...(Args) - 2), extractData<T2, Args...>()); }
	// extracts the contents of the structure in the form of a tuple on the stack, according to the explicit types passed
	template<typename T> std::tuple<T> extract() { return peekTuple<T>(members - 1); }

private:
	unique<shared<void>[]> data;
	size_t size = 0;
	template<typename T> inline std::tuple<T> peekTuple(const size_t index) { return std::tuple<T>(peek(index)); }
};