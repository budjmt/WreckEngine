#pragma once

#include <condition_variable>
#include <mutex>
#include <shared_mutex>
#include <queue>
#include <map>

#include "smart_ptr.h"

// thread-safe queue that can be populated/depopulated from multiple threads
template<typename T, typename Container = std::deque<T>>
class safe_queue {
public:

    void swap(safe_queue& other) {
        using std::swap;
        swap(mut, other.mut);
        swap(condition, other.condition);
        swap(queue, other.queue);
    }

    void push(const T& value) {
        {
            std::unique_lock<std::mutex> lock(mut);
            queue.push(value);
        }
        condition.notify_one();
    }

    void push(T&& value) {
        {
            std::unique_lock<std::mutex> lock(mut);
            queue.push(std::move(value));
        }
        condition.notify_one();
    }

    T& emplace(T&& value) {
        T* el;
        {
            std::unique_lock<std::mutex> lock(mut);
            el = &queue.emplace(std::move(value));
        }
        condition.notify_one();
        return *el;
    }

    T pop() {
        std::unique_lock<std::mutex> lock(mut);
        condition.wait(lock, [this] { return !empty(); });
        T front(std::move(queue.front()));
        queue.pop();
        return front;
    }

    template<typename Duration>
    bool tryPop(T& outVal, Duration duration) {
        std::unique_lock<std::mutex> lock(mut);
        if (!condition.wait_for(lock, duration, [this] { return !empty(); })) {
            return false;
        }
        outVal = std::move(queue.front());
        queue.pop();
        return true;
    }

    auto size() const { return queue.size(); }
    bool empty() const { return queue.empty(); }

private:
    std::mutex mut;
    std::condition_variable condition;
    std::queue<T, Container> queue;
};

// simple data structure designed for a list that's populated once, and can only be repopulated once consumed
// designed to be usable by one consumer, one producer
template<typename T, size_t frame_cache = 2>
class frame_vector {
public:
    std::vector<T>& get() {
        return frameLists[activeList];
    }

    // call at the end of a frame to indicate that the cache should be moved to the next frame
    void seal() {
        ++activeList %= frame_cache;
        if (++numSealed == frame_cache) {
            Thread::spinUntil([this] { return numSealed < frame_cache || Window::closing(); }, 0.25f);
        }
    }

    // [func] is a function that iterates over the list, "consuming" it
    void consume(const std::function<void(std::vector<T>&)>& func) {
        Thread::spinUntil([this] { return numSealed > 0 || Window::closing(); }, 0.25f);
        if (numSealed == 0) return;

        auto& oldest = getOldest();
        func(oldest);
        oldest.clear();

        --numSealed;
    }
private:
    std::vector<T> frameLists[frame_cache];
    size_t activeList = 0;
    std::atomic<size_t> numSealed = 0;

    std::vector<T>& getOldest() {
        return frameLists[(frame_cache + activeList - numSealed) % frame_cache];
    }
};

template<typename T, size_t frame_cache = 2>
class thread_frame_vector {
public:
    auto& getFrameVector() {
        return vectors[std::this_thread::get_id()];
    }

    // returns the active vector corresponding to the current thread
    std::vector<T>& get() {
        return getFrameVector().get();
    }

    void seal() { 
        const auto id = std::this_thread::get_id();
        if (auto vec = vectors.find(id); vec != end(vectors)) vec->second.seal();
    }

    // consumes all threads' frame_vectors one at a time
    void consumeAll(const std::function<void(std::vector<T>&)>& func) { 
        for (auto& frameVec : vectors) {
            frameVec.second.consume(func);
        }
    }

    // resets the thread map; intended for use after threads that don't normally access the queue do so to improve efficiency and stability
    void flush() { vectors.clear(); }
private:
    std::map<std::thread::id, frame_vector<T, frame_cache>> vectors;
};