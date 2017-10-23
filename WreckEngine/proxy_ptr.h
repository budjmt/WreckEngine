#pragma once

#include <cassert>
#include <unordered_map>

/*------------------------------------------
PROXY POINTERS

PURPOSE
 - provide a layer of indirection over a pointer to some buffer that 
   allows the underlying buffer to be reallocated
 - allow for data-oriented buffers that can be reallocated under the hood 
   without external code worrying about ptr validity
 - allow for references to items in a sorted buffer pre-sorting
 
REQUIREMENTS
 - thread-safety; can be opt-in
 - using pointer arithmetic accesses expected memory in the underlying buffer
 - underlying buffer can allocate on cache lines
 
IDEAS
 - Proxy pointers request access to grab the pointer in any way
   - deref MUST copy
   - getting the pointer goes through the handle mapping
     - it must lock to be thread-safe
 - implement the arithmetic operators
   - it would be excessive overhead to manager every pointer in a buffer
     - proxies store the handle, which refers to the start of the buffer
     - they also store an offset, which is added to the pointer retrieved from the mapping

------------------------------------------*/

template<typename T>
class proxy_buffer;

template<typename T>
struct proxy_ptr {
    using proxy_map = std::unordered_map<int, proxy_buffer<T>&>;

    proxy_ptr() = default;
    proxy_ptr(int _handle, int offset, const proxy_map* _owner = nullptr) : handle(_handle), offsetBytes(offset * sizeof(T)), owner(_owner) {}

    proxy_ptr& operator++() { offsetBytes += sizeof(T); return *this; }
    proxy_ptr operator++(int) { auto pre = *this; offsetBytes += sizeof(T); return pre; }
    proxy_ptr& operator--() { offsetBytes -= sizeof(T); return *this; }
    proxy_ptr operator--(int) { auto pre = *this; offsetBytes -= sizeof(T); return pre; }

    proxy_ptr& operator+=(int offset) { offsetBytes += offset * sizeof(T); return *this; };
    proxy_ptr& operator-=(int offset) { offsetBytes -= offset * sizeof(T); return *this; };
    proxy_ptr operator+(int offset) const { auto p = *this; return p += offset; };
    proxy_ptr operator-(int offset) const { auto p = *this; return p -= offset; };

    template<typename = std::enable_if<std::is_copy_constructible<T>::value>>
    T operator*() const { return *get(); }
    T* operator->() const { return get(); }

    operator bool() { return get() != nullptr; }

private:
    // optionally replace ternary with assert
    // this can also be public for single thread
    T* get() const { return (owner && owner->count(handle)) ? (T*)((uint8_t*)(owner->at(handle).data()) + offsetBytes) : nullptr; }

    int handle;
    int offsetBytes = 0;
    const proxy_map* owner = nullptr;
};

template<typename T> using proxy = proxy_ptr<T>;

// stores handles to proxy_buffers, which store the data proxy_ptrs point to
template<typename T>
class proxy_handler {
public:
    using map_t = typename proxy<T>::proxy_map;

    int track(proxy_buffer<T>& buffer) { 
        // uncomment if wrap-around is a concern (~4 billion buffers of a given type)
        //while (mapping.count(numHandles)) ++numHandles;
        mapping.insert({ numHandles, buffer });
        return numHandles++; 
    }
    void erase(int handle) { mapping.erase(handle); }

    const map_t* get() const { return &mapping; }

private:
    int numHandles = 1;
    map_t mapping;
};

template<typename T>
class proxy_buffer {
public:
    proxy_buffer(size_t capacity = 0) : handleMap(get_handler()) { 
        buffer.reserve(capacity); 
        handle = handleMap->track(*this); 
    }
    ~proxy_buffer() { handleMap->erase(handle); }

    proxy_buffer(const proxy_buffer<T>& other) : buffer(other.buffer) {
        handleMap = other.handleMap;
        handle = handleMap->track(*this);
    }

    proxy_buffer<T>& operator=(const proxy_buffer<T>& other) {
        buffer = other.buffer;
        return *this;
    }

    proxy_buffer(proxy_buffer<T>&& other) : buffer(other.buffer) {
        handleMap = other.handleMap;
        handle = handleMap->track(*this);
    }

    proxy_buffer<T>& operator=(proxy_buffer<T>&& other) {
        buffer = std::move(other.buffer);
        return *this;
    }

    proxy<T> get(int index = 0) const { return { handle, index, handleMap->get() }; }
    proxy<T> operator()(size_t index) const { return get(index); }
    // this is intended for modifications of elements; only the buffer itself has permission to do that
    T& operator[](size_t index) { return buffer[index]; }
    const T& operator[](size_t index) const { return buffer[index]; }

    T* data() { return buffer.data(); }
    const T* data() const { return buffer.data(); }
    size_t size() const { return buffer.size(); }

    void reserve(size_t capacity) { buffer.reserve(capacity); }
    void resize(size_t count) { buffer.resize(count); }

    void push_back(T t) { buffer.push_back(t); }

    template<typename... Args>
    void emplace_back(Args&&... args) { buffer.emplace_back(std::forward<Args>(args)...); }

private:
    std::vector<T> buffer;
    int handle = -1;
    shared<proxy_handler<T>> handleMap; // this prevents the handler from being destroyed until all relevant buffers are as well
    static shared<proxy_handler<T>> get_handler() {
        static shared<proxy_handler<T>> handler = make_shared<proxy_handler<T>>();
        return handler;
    }
};