#include "DebugBenchmark.h"

DebugBenchmark& DebugBenchmark::getInstance() {
	static DebugBenchmark instance;
	return instance;
}

void DebugBenchmark::start() {
	timer = glfwGetTime();
}

//numbers of milliseconds since timer start
double DebugBenchmark::end() {
	double time = glfwGetTime() - timer;
	return time * 1000;
}