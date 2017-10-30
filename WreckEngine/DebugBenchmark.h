#pragma once

#include "Time.h"

class DebugBenchmark
{
public:
    static void start() { 
        timer = Time::now();
    }

    static double end() { 
        auto end = Time::now();
        auto time = Time::get_duration(timer, end);
        return time * 1000;
    }
private:
    static thread_local Time::time_point timer;
};

thread_local Time::time_point DebugBenchmark::timer = Time::now();
