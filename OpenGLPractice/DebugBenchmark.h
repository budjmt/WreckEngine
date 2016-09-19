#pragma once

#include "GLFW/glfw3.h"

class DebugBenchmark
{
public:
	static void start() { 
		timer = glfwGetTime(); 
	}

	static double end() { 
		auto time = glfwGetTime() - timer;
		return time * 1000;
	}
private:
	static double timer;
};

double DebugBenchmark::timer = glfwGetTime();