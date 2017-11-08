#pragma once

#include <functional>

#include "Time.h"
#include "property.h"

// a value designed to be used in a single thread that is computed once per frame
// it updates any time the value is accessed on a later frame than it was last updated
// it must only be used on one thread as the frame count value differs per update thread
// this version calls a function from the parent object on its value and allows the parent to access the value directly
template<typename T, typename P = void>
class frame_cache : public detail::parent_ptr<P> {
public:
    using update_t = void (P::*)(T&);
    frame_cache(P* _parent, update_t update_func) : parent_ptr(_parent), update(update_func) {}

    const T& operator()() {
        if (auto currFrame = Time::frameCount; currFrame > lastUpdateFrame) {
            lastUpdateFrame = currFrame;
            (parent->*update)(value);
        }
        return value;
    }
private:
    size_t lastUpdateFrame = 0;
    T value;
    update_t update;

    friend P;
    T& operator*() { return value; }
    T* operator->() { return &value; }
};

// a value designed to be used in a single thread that is computed once per frame
// it updates any time the value is accessed on a later frame than it was last updated
// it must only be used on one thread as the frame count value differs per update thread
// this version calls some callable object on its value
template<typename T>
class frame_cache<T, void> {
public:
    using update_t = std::function<void(T&)>;
    frame_cache(update_t update_func) : update(update_func) {}

    const T& operator()() {
        if (auto currFrame = Time::frameCount; currFrame > lastUpdateFrame) {
            lastUpdateFrame = currFrame;
            update(value);
        }
        return value;
    }
private:
    size_t lastUpdateFrame = 0;
    T value;
    update_t update;
};
