#pragma once

#include <unordered_map>
#include <unordered_set>
#include <string>

#include <cassert>

#include "Random.h"

#define UNIQUE_NAMES(access, type) \
static uint32_t get(const std::string& name) { return names.get(name); } \
static uint32_t add(const std::string& name) { return names.add(name); } \
access: static unique_name<type> names; public: 

template<typename T>
class unique_counter {
protected: 
	static inline uint32_t& counter() { static uint32_t c = Random::get(); return c; }
public:
	uint32_t operator++() { return ++counter(); };
	uint32_t operator++(int) { return counter()++; };
};

// the template is so that a unique set is generated for each class using it
template<typename T>
class unique_name : public unique_counter<T> {
public:
	static uint32_t get(const std::string& name) { 
		assert(ids.count(name)); // the name wasn't added before it was used
		return ids.at(name); 
	}

	static uint32_t add(const std::string& name) {
		const auto insertInfo = ids.insert({ name, counter() + 1 });
		if (!insertInfo.second) throw "The name \"" + name + "\" is already in use; ID: " + std::to_string(ids[name]);
		return ++counter();
	}

private:
	static std::unordered_map<std::string, const uint32_t> ids;
};

template<typename T> std::unordered_map<std::string, const uint32_t> unique_name<T>::ids;

// use this macro instead of the body, since it's subject to change
#define UNIQUE_TYPE_ID(type) unique_type::id<type>()

#define PARENT_TYPE(child_t, parent_t) template class unique_type::parent_index<parent_t, child_t>

#define CHILD_TYPE(parent_t, child_t) PARENT_TYPE(child_t, parent_t)

class unique_type {
public:
	struct data {
		std::unordered_set<uint32_t> child_ids;
	};

	template<typename T>
    static uint32_t id() { static const uint32_t _id = unique_type::register_type(); return _id; }

	// do not use this in any code, it's just used to create parent-child relationships in the macro
	// a slower, more flexible exceptions-based approach: http://stackoverflow.com/questions/15832679/how-to-get-a-type-from-type-info-for-template-parameters
	template<typename P, typename C>
	class parent_index {
		static const int add_result;
	};

	// this is done instead of a static field to ensure initialization
	static inline std::unordered_map<uint32_t, data>& registry() { static std::unordered_map<uint32_t, data> r; return r; };

	static inline data& get_data(const uint32_t id) { return registry().at(id); }

private:
	static uint32_t register_type() {
		auto counter = ++unique_counter<unique_type>();
		registry().insert({ counter, data() }); 
		return counter;
	}

	static inline int add_child(const uint32_t parent, const uint32_t child) { registry().at(parent).child_ids.insert(child); return 0; }
};

template<typename P, typename C> const int unique_type::parent_index<P, C>::add_result = unique_type::add_child(id<P>(), id<C>());