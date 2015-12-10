#pragma once

#include "GLFW/glfw3.h"

class DebugBenchmark
{
public:
	static DebugBenchmark& getInstance();

	void start(); 
	double end();
private:
	DebugBenchmark() { timer = glfwGetTime(); };
	//~DebugBenchmark();
	DebugBenchmark(const DebugBenchmark&) = delete;
	void operator=(const DebugBenchmark&) = delete;

	double timer;
};

