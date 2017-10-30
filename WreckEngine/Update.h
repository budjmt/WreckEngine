#pragma once

#include <functional>
#include <thread>
#include <vector>

#include "Time.h"
#include "External.h"

struct UpdateBase {

    explicit UpdateBase(std::thread* thread) { updateThreads.push_back(thread); }
    static void join() {
        for (auto thread : updateThreads) thread->join(); 
    }

protected:
    static std::mutex mut;
    static std::condition_variable exitCondition;

    void waitForWindowFocus() {
        if (Time::frameBegin > Window::focusLostTime) return;
        
        auto end = Time::now();
        auto pauseOverlapWithFrame = std::max(Time::get_duration(Window::focusLostTime, end), 0.0);

        Thread::spinUntil([] { return Window::isInFocus || Window::closing(); }, 0.25f);

        auto pauseLength = Time::get_duration(Window::focusLostTime, Time::now());
        Time::nextDeltaOffset = pauseOverlapWithFrame - pauseLength;
    }

private:
    static std::vector<std::thread*> updateThreads;
};

// creates a thread that will run all functions it is constructed with on the interval specified: [freq] iterations per [sec] seconds
template<size_t freq, size_t sec = 1>
class Update : public UpdateBase {
public:

    Update(std::function<void()> updateFunc, std::function<void()> initFunc = []() {}) : UpdateBase(&thread), init(initFunc), update(updateFunc) {}

    void run() {
        init();
        while (!Window::closing()) {
            Time::update();
            auto next = Time::now() + interval;

            update();

            {
                std::unique_lock<std::mutex> lock(mut);
                exitCondition.wait_until(lock, next, [] { return Window::closing(); });
            }

            waitForWindowFocus();
        }
    }

private:
    static constexpr auto interval = std::chrono::duration<double>((double) sec / freq);
    std::function<void()> init, update;
    std::thread thread = std::thread([this] { this->run(); });
};

// specialization that runs as often as possible
template<>
class Update<0> : public UpdateBase {
public:

    Update(std::function<void()> updateFunc, std::function<void()> initFunc = []() {}) : UpdateBase(&thread), init(initFunc), update(updateFunc) {}

    void run() {
        init();
        while (!Window::closing()) {
            Time::update();
            update();
            waitForWindowFocus();
        }
    }

private:
    std::function<void()> init, update;
    std::thread thread = std::thread([this] { this->run(); });
};