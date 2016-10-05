#pragma once

#include <unordered_map>
#include <string>

#define UNIQUE_NAMES(access, type) \
static uint32_t get(const std::string name) { return names.get(name); } \
static uint32_t add(const std::string name) { return names.add(name); } \
access: static unique_name<type> names; public: 

template<typename T>
class unique_counter { protected: static uint32_t counter; };

template<typename T> uint32_t unique_counter<T>::counter = rand();

// the template is so that a unique set is generated for each class using it
template<typename T>
class unique_name : public unique_counter<T> {
public:
	static uint32_t get(const std::string name) { return ids.at(name); }

	static uint32_t add(const std::string name) {
		const auto res = ids.insert({ name, counter + 1 });
		if (!res.second) throw "The event name \"" + name + "\" is already in use; ID: " + std::to_string(ids[name]);
		return ++counter;
	}

private:
	static std::unordered_map<std::string, const uint32_t> ids;
};

template<typename T> std::unordered_map<std::string, const uint32_t> unique_name<T>::ids;

template<typename T>
class unique_type : public unique_counter<T> {
public:
	static const uint32_t id;
	operator uint32_t() { return id; }
};

template<typename T> const uint32_t unique_type<T>::id = ++counter;