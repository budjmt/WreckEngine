#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"

#include "GLDebug.h"
#include "ShaderHelper.h"
#include "ModelHelper.h"
#include "Mouse.h"
#include "TriPlay.h"

using namespace std;

//DEBUG is defined in DrawDebug.h

double FPS = 60;

GLFWwindow* window;
GLuint shaderProg;
GLint uniTime;

double prevFrame;
Mouse* mouse;

TriPlay* game;

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void mouse_move_callback(GLFWwindow* window, double x, double y);

void init() {
	shaderProg = loadShaderProgram("Shaders/matvertexShader.glsl","Shaders/matfragmentShader.glsl");
	if(shaderProg != 0) {
		glUseProgram(shaderProg);
		setShaderColor(shaderProg,"tint",1,1,1);
		uniTime = glGetUniformLocation(shaderProg, "time");
	}

	mouse = new Mouse();
	mouse_move_callback(window, 0, 0);//this is cheating but it works

	game = new TriPlay(shaderProg, window);
}

void initGraphics(GLFWwindow* window) {
	int width, height;
	glfwGetWindowSize(window, &width, &height);

	float aspect = (float)width / (float)height;
	glViewport(0, 0, width, height);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//wraps uvs
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	/*
	//back-face culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	*/

	//cornflower blue
	glClearColor(0.392f, 0.584f, 0.929f, 1.0f);
	//glClearColor(0.f, 0.f, 0.f, 1.f);

	//render as wireframe
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

#include <iostream>

void update() {
	double currFrame = glfwGetTime();
	double dt = currFrame - prevFrame;
	prevFrame = currFrame;
	//need to separate drawing and update pipelines
	float spf = 1.f / FPS;
	while (dt < spf) {
		currFrame = glfwGetTime();
		dt += currFrame - prevFrame;
		prevFrame = currFrame;
	}

	mouse->prevx = glm::mix(mouse->prevx, mouse->x, 0.15f);
	mouse->prevy = glm::mix(mouse->prevy, mouse->y, 0.15f);
	
	bool alt = glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS;
	if (alt) {
		if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS)
			FPS += (FPS < 5) ? 1 : ((FPS < 20) ? 5 : ((FPS < 60) ? 10 : 0));
		if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS)
			FPS -= (FPS > 20) ? 10 : ((FPS > 5) ? 5 : ((FPS > 1) ? 1 : 0));
		std::cout << "FPS is now " << FPS << std::endl;
	}

	glUniform1f(uniTime, currFrame);
	game->update(window, mouse, dt);
}

void draw() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	game->draw();

	glFlush();
}

int main(int argc, char** argv) {
	if (!glfwInit())
		return -1;

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	
	/*const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);*/

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	if(DEBUG)
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

	window = glfwCreateWindow(800, 600, "OpenGL App", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}

	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, mouse_move_callback);

	glfwMakeContextCurrent(window);

	glewExperimental = GL_TRUE;

	if (glewInit() != GLEW_OK)
		return -1;

	if(DEBUG)
		initDebug();
	init();
	initGraphics(window);
	while (!glfwWindowShouldClose(window)) {
		update();
		draw();

		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	delete mouse;
	glfwTerminate();
	return 0;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	mouse->button = button;
	if (action == GLFW_PRESS) {
		//cout << "Mouse: " << mousex << "," << mousey << endl;
		//entities[0].setPos(glm::vec3(mousex,mousey,0));
		mouse->down = true;
		mouse->lastClick = glfwGetTime();
	}
	else if (action == GLFW_RELEASE) {
		mouse->down = false;
	}
}

void mouse_move_callback(GLFWwindow* window, double x, double y) {
	double cursorx, cursory;
	int windowWidth, windowHeight;
	glfwGetCursorPos(window, &cursorx, &cursory);
	glfwGetWindowSize(window, &windowWidth, &windowHeight);
	mouse->prevx = mouse->x;
	mouse->prevy = mouse->y;
	mouse->x = 2 * cursorx / windowWidth - 1;
	mouse->y = (2 * cursory / windowHeight - 1) * -1;
}