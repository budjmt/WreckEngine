#pragma once

#include <chrono>

namespace Time {

    extern std::chrono::high_resolution_clock::time_point start;
    extern thread_local std::chrono::high_resolution_clock::time_point prevFrame;
    extern thread_local double delta;

    std::chrono::high_resolution_clock::time_point now();
    long long ctime();
    double elapsed();
    void updateDelta();
};