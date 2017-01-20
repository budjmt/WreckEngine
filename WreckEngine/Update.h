#pragma once

#include <functional>
#include <thread>
#include <vector>

#include "Time.h"
#include "External.h"

struct UpdateBase {

    UpdateBase(std::thread* thread) { updateThreads.push_back(thread); }
    static void join() { for (auto thread : updateThreads) thread->join(); }

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
            Time::updateDelta();
            auto next = Time::now() + interval;

            update();

            std::this_thread::sleep_until(next);
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
            Time::updateDelta();
            update();
        }
    }

private:
    std::function<void()> init, update;
    std::thread thread = std::thread([this] { this->run(); });
};