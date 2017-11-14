#pragma once

#include <vector>

template<typename T>
class alias_vector : public std::vector<T> {
public:
    T& at(size_t index) { return *(T*)((char*)data() + objSize * index); }
    T& operator[](size_t index) { return at(index); }

    const T& at(size_t index) const { return *(T*)((char*)data() + objSize * index); }
    const T& operator[](size_t index) const { return at(index); }
private:
    size_t objSize = sizeof(T);
};

template<> class alias_vector<bool> {}; // disabled because vector<bool> is to much of an aberration