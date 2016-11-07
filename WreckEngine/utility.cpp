#include "gl_structs.h"
#include "External.h"

// this file is used to define any external stuff needed by all the various utility files

GLint GLtexture::MAX_TEXTURES = -1;

GLFWwindow* Window::window;
int Window::width;
int Window::height;
float Window::aspect;
int Window::cursorMode = GLFW_CURSOR_NORMAL;

Mouse::Info Mouse::info;

void Mouse::update() {
	info.prev.x = glm::mix(info.prev.x, info.curr.x, 0.15);
	info.prev.y = glm::mix(info.prev.y, info.curr.y, 0.15);
}

void Mouse::default_button(GLFWwindow* window, int button, int action, int mods) {
	if (action == GLFW_PRESS) {
		Mouse::info.setButtonDown(button);
		Mouse::info.buttons[button].lastDown = glfwGetTime();
	}
	else if (action == GLFW_RELEASE) {
		Mouse::info.setButtonUp(button);
	}
}

void Mouse::default_move(GLFWwindow* window, double x, double y) {
	double cursorx, cursory;
	// retrieves the mouse coordinates in screen-space, relative to top-left corner
	glfwGetCursorPos(window, &cursorx, &cursory);
	Mouse::info.prev = Mouse::info.curr;
	Mouse::info.curr.x =   2 * cursorx - 1;
	Mouse::info.curr.y = -(2 * cursory - 1);
}