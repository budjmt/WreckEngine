#pragma once

#include "GLFW/glfw3.h"

class DebugBenchmark
{
public:
	static void start() { 
		timer = Time::elapsed(); 
	}

	static double end() { 
		auto time = Time::elapsed() - timer;
		return time * 1000;
	}
private:
	static double timer;
};

double DebugBenchmark::timer = Time::elapsed();