#pragma once

#include "GLFW/glfw3.h"
#include "glm/glm.hpp"

struct Window {
	static GLFWwindow* window;
	static int width, height;
	static float aspect;

	static inline void update() { glfwGetWindowSize(window, &width, &height); aspect = (float)width / (float)height; }
	static inline int getKey(const int keyCode) { return glfwGetKey(window, keyCode); }
};

struct Mouse {
	// this style enables copies to be made, for better concurrency
	struct Info {
		struct button { double lastDown = 0; };
		struct cursor { double x = 0, y = 0; };

		uint32_t down = 0;// bit-field
		button buttons[3];
		cursor curr, prev;
		const double clickCoolDown = 0.2;

		inline uint32_t getButtonState(int b) { return down & (1 << b); }
		inline void setButtonDown(int b) { down |= 1 << b; }
		inline void setButtonUp(int b) { down &= ~(1 << b); }
	};
	static Info info;

	static void update();

	static inline void button_callback(GLFWmousebuttonfun f) { glfwSetMouseButtonCallback(Window::window, f); }
	static inline void move_callback(GLFWcursorposfun f)     { glfwSetCursorPosCallback(Window::window, f); }

	static void default_button(GLFWwindow* window, int button, int action, int mods);
	static void default_move(GLFWwindow* window, double x, double y);
};