#pragma once

#include "Time.h"

#include <random>

// TODO fix with actual random generator
namespace Random {

    struct well512 {
        typedef uint32_t result_type;
        static constexpr result_type min() { return 0; }
        static constexpr result_type max() { return UINT32_MAX; }

        static result_type state[16];
        static result_type seed(result_type seedVal);
        result_type operator()();
    };

    //period 2^128-1
    struct xorshift128plus {
        typedef uint64_t result_type;
        static constexpr result_type min() { return 0; }
        static constexpr result_type max() { return UINT64_MAX; }

        static result_type state[2];
        static result_type seed(result_type seedVal);
        result_type operator()();
    };

    typedef well512 engine_t;
    extern thread_local engine_t engine;

	inline auto seed() {
        return engine_t::seed((engine_t::result_type) Time::ctime());
    }

	inline unsigned get() {
        static auto do_seed = seed();
        static std::uniform_int_distribution<engine_t::result_type> dist(engine_t::min(), engine_t::max());
        return dist(engine);
    }
    inline int getRange(engine_t::result_type start, engine_t::result_type end) { return std::uniform_int_distribution<engine_t::result_type>(start, end)(engine); }
	inline int getRangeFast(engine_t::result_type start, engine_t::result_type end) { return get() % (end - start) + start; }

    inline float  getf() { return std::uniform_real_distribution<float> (0.f, 1.f)(engine); }
    inline double getd() { return std::uniform_real_distribution<double>(0.0, 1.0)(engine); }

	inline float  getfFast() {
        constexpr float  inv_max_range = 1.f / engine_t::max();
        return get() * inv_max_range;
    }
	inline double getdFast() {
        constexpr double inv_max_range = 1. / engine_t::max();
        return get() * inv_max_range;
    }

    // TODO incorporate lib noise
    namespace Coherent {
    }
};