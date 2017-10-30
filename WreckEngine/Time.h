#pragma once

#include <chrono>

namespace Time {

    using time_point = std::chrono::high_resolution_clock::time_point;

    extern const time_point start;
    extern thread_local const time_point& frameBegin;
    extern thread_local const double& delta; // elapsed seconds since last frame in the current thread, not counting time not in focus
    extern thread_local double nextDeltaOffset; // value used to offset the next frame's delta time in case of pausing or other aberrations
    extern thread_local const double& elapsed; // elapsed seconds since game start at the beginning of the frame in the current thread, not counting time not in focus

    time_point now();
    std::chrono::system_clock::time_point system_now();
    long long ctime();

    template<typename T = double>
    inline T get_duration(time_point start, time_point end) { 
        using namespace std::chrono;
        return duration<T>(end - start).count();
    }
    void update();
};