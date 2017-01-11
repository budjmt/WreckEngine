#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>
#include <map>

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

    void emplace(T&& value) {
        {
            std::unique_lock<std::mutex> lock(mut);
            queue.emplace(std::move(value));
        }
        condition.notify_one();
    }

    T pop() {
        std::unique_lock<std::mutex> lock(mut);
        condition.wait(lock, [this]() { return !empty(); });
        T front(std::move(queue.front()));
        queue.pop();
        return front;
    }

    bool tryPop(T& outVal, std::chrono::milliseconds duration) {
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
template<typename T>
class frame_vector : public std::vector<T> {
public:
    void push_back(const T& value) {
        if (isSealed) return;
        std::vector<T>::push_back(value);
    }

    void push_back(T&& value) { 
        if (isSealed) return; 
        std::vector<T>::push_back(std::move(value)); 
    }

    template<typename... Args>
    void emplace_back(Args&&... args) { 
        if (isSealed) return; 
        std::vector<T>::emplace_back(std::forward<Args>(value)...); 
    }

    void seal() {
        if (isSealed) return;
        std::unique_lock<std::mutex> lock(mut);
        isSealed = true;
        condition.notify_one();
    }

    void unseal() {
        if (!isSealed) return;
        std::unique_lock<std::mutex> lock(mut);
        clear();
        isSealed = false;
    }

    // [func] is a function that iterates over the list, "consuming" it
    void consume(std::function<void(frame_vector<T>&)> func) {
        std::unique_lock<std::mutex> lock(mut);
        condition.wait(lock, [this] { return isSealed; });
        func(*this);
    }
private:
    bool isSealed = false;
    std::mutex mut;
    std::condition_variable condition;
};

template<typename T>
class thread_frame_vector {
public:
    // returns the frame_vector corresponding to the current thread
    frame_vector<T>& get() { return vectors[std::this_thread::get_id()]; }
    
    // consumes all threads' frame_vectors one at a time
    void consumeAll(std::function<void(frame_vector<T>&)> func) { for(auto& frameVec : vectors) frameVec.second.consume(func); }

    // use when threads that aren't supposed to be producing do so and you want to reset the map
    void flush() { vectors.clear(); }
private:
    std::map<std::thread::id, frame_vector<T>> vectors;
};