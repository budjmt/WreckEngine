#pragma once

#include "gl_structs.h"

// only use this class on a GL context thread
class DebugGPUBenchmark {
public:
    static void start() {
        thread_local struct X {
            X() { query.create(GLquery::Target::TimeElapsed); }
        } init;

        query.execute();
    }

    static double end() {
        query.finish();
        auto elapsed = query.getResult(); // returns nanoseconds
        return elapsed * 10e6;
    }
private:
    static thread_local GLquery query;
};

thread_local GLquery DebugGPUBenchmark::query;