#include "Random.h"

using namespace Random;

thread_local engine_t Random::engine;

bool well512::seeded = false;
well512::result_type well512::state[16];

well512::result_type well512::seed(result_type seedVal) {
    seeded = true;
    for (size_t i = 0; i < 16; ++i)
        state[i] = (seedVal >> i) * seedVal;
    return seedVal;
}

well512::result_type well512::operator()() {
    static uint32_t index = 0;

    result_type a, b, c, d;

    a = state[index];
    c = state[(index + 13) & 15];
    b = a ^ c ^ (a << 16) ^ (c << 15);

    c = state[(index + 9) & 15];
    c ^= (c >> 11);
    a = state[index] = b^c;
    d = a ^ ((a << 5) & 0xDA442D24);

    index = (index + 15) & 15;
    a = state[index];
    state[index] = a ^ b ^ d ^ (a << 2) ^ (b << 18) ^ (c << 28);
    return state[index];
}

bool xorshift128plus::seeded = false;
xorshift128plus::result_type xorshift128plus::state[2];

xorshift128plus::result_type xorshift128plus::seed(result_type seedVal) {
    seeded = true;
    state[0] = (seedVal & 0x00000000ffffffff) * seedVal;
    state[1] = (seedVal & 0xffffffff00000000) * seedVal;
    return seedVal;
}

xorshift128plus::result_type xorshift128plus::operator()() {
    auto x = state[0];
    const auto y = state[1];
    state[0] = y;

    x ^= x << 23;
    state[1] = x ^ y ^ (x >> 17) ^ (y >> 26);
    return state[1] + y;
}