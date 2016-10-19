#pragma once

#include <cstdlib>
#include <ctime>

// TODO fix with actual random generator
struct Random {
	static auto seed() { auto s = time(NULL); srand((unsigned long)s); return s; }

	static unsigned get() { static auto do_seed = seed(); return rand(); }
	static int getRange(int start, int end) { return get() % (end - start) + start; }

	static float  getf() { constexpr unsigned max_range = 1 << 24; return (get() & (max_range - 1)) * (1.f / max_range); }
	static double getd() { constexpr unsigned max_range = 1 << 24; return (get() & (max_range - 1)) * (1. / max_range); }
};