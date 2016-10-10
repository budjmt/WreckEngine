#pragma once

#include "GLFW/glfw3.h"
#include "glm/glm.hpp"

struct Window {
	static GLFWwindow* window;
	static int width, height;
	static float aspect;

	static void update() { glfwGetWindowSize(window, &width, &height); aspect = (float)width / (float)height; }
	static int getKey(const int keyCode) { return glfwGetKey(window, keyCode); }
};

struct Mouse {
	// this style enables copies to be made, for better concurrency
	struct Info {
		int button = 0;
		double x = 0, y = 0, prevx = 0, prevy = 0;
		bool down = false;
		double clickCoolDown = 0.2, lastClick = 0;
	};
	static Info info;

	static void update() {
		info.prevx = glm::mix(info.prevx, info.x, 0.15f);
		info.prevy = glm::mix(info.prevy, info.y, 0.15f);
	}

	static void button_callback(GLFWmousebuttonfun f) { glfwSetMouseButtonCallback(Window::window, f); }
	static void move_callback(GLFWcursorposfun f) { glfwSetCursorPosCallback(Window::window, f); }
};