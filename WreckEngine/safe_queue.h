#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>

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