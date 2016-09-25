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

template<class T>
using shared = std::shared_ptr<T>;

template<class T >
using weak = std::weak_ptr<T>;

template<class T>
using alloc = alloc_ptr<T>;