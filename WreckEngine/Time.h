#pragma once

#include <chrono>

namespace Time {

    extern const std::chrono::high_resolution_clock::time_point start;
    extern thread_local const double& delta;

    std::chrono::high_resolution_clock::time_point now();
    std::chrono::system_clock::time_point system_now();
    long long ctime();
    double elapsed();
    void updateDelta();
};