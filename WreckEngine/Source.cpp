
#include <vld.h>

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"

#include "GLDebug.h"
#include "ShaderHelper.h"
#include "ModelHelper.h"
#include "External.h"
#include "TriPlay.h"

using namespace std;

void initGraphics();

struct GLFWmanager { 
	bool initialized = false; 
	~GLFWmanager() { if(initialized) glfwTerminate(); };
	GLFWmanager(const size_t width, const size_t height) { 
		auto val = glfwInit(); 
		initialized = val != 0; 
		if(!initialized) exit(val); 

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

		GLFWmonitor* monitor = glfwGetPrimaryMonitor();

		/*const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		glfwWindowHint(GLFW_RED_BITS, mode->redBits);
		glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
		glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
		glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);*/

		if (DEBUG) glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

		Window::window = glfwCreateWindow(width, height, "Wreck Engine", nullptr, nullptr);
		if (!Window::window) exit('w');
		glfwMakeContextCurrent(Window::window);
		Window::update();

		Mouse::button_callback(Mouse::default_button);
		Mouse::move_callback(Mouse::default_move);

		GLtexture::setMaxTextures();
	};
};

struct GLEWmanager {
	bool initialized = false;
	GLEWmanager() {
		glewExperimental = GL_TRUE;
		auto val = glewInit();
		initialized = val == GLEW_OK;
		if (!initialized) exit(val);
	}
};

//DEBUG is defined in DrawDebug.h

double FPS = 60;
double runningAvgDelta = 1.0 / FPS;
int samples = 10;
bool fpsMode = true;

GLprogram shaderProg;

double prevFrame;

unique<TriPlay> game;

void init() {
	shaderProg = loadProgram("Shaders/matvertexShader.glsl","Shaders/matfragmentShader.glsl");
	if(shaderProg()) {
		shaderProg.use();
		shaderProg.getUniform<vec4>("tint").update(vec4(1));
	}

	prevFrame = glfwGetTime();
	Mouse::default_move(Window::window, 0, 0);//this is cheating but it works for initializing the mouse

	initGraphics();

	Text::init();
	game = make_unique<TriPlay>(shaderProg);// this won't be initialized until after GLFW/GLEW are
}

void initGraphics() {
	glViewport(0, 0, Window::width, Window::height);

	// alpha blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	// texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//wraps UVs
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// depth buffering
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	// back-face culling
	if (!DEBUG) { glEnable(GL_CULL_FACE); }
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	// render as wire-frame
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

#include <iostream>

void update() {
	auto currFrame = glfwGetTime();
	auto dt = currFrame - prevFrame;
	prevFrame = currFrame;
	//need to separate drawing and update pipelines
	//double spf = 1.0 / FPS;
	//while (dt < spf) {
	//	currFrame = glfwGetTime();
	//	dt += currFrame - prevFrame;
	//	prevFrame = currFrame;
	//}

	runningAvgDelta -= runningAvgDelta / samples;
	runningAvgDelta += dt / samples;
	auto title = std::to_string(fpsMode ? 1.0 / runningAvgDelta : runningAvgDelta * 1000.0);
	auto decimal = title.find('.', 2);
	if (title.length() - decimal > 3) title = title.erase(decimal + 3);
	title += fpsMode ? " FpS" : " MSpF";
	glfwSetWindowTitle(Window::window, title.c_str());

	Mouse::update();

	bool alt = Window::getKey(GLFW_KEY_RIGHT_ALT) == GLFW_PRESS || Window::getKey(GLFW_KEY_LEFT_ALT) == GLFW_PRESS;
	if (alt) {
		if (Window::getKey(GLFW_KEY_0) == GLFW_PRESS)
			fpsMode = !fpsMode;
		if (Window::getKey(GLFW_KEY_EQUAL) == GLFW_PRESS)
			FPS += (FPS < 5) ? 1 : ((FPS < 20) ? 5 : ((FPS < 60) ? 10 : 0));
		if (Window::getKey(GLFW_KEY_MINUS) == GLFW_PRESS)
			FPS -= (FPS > 20) ? 10 : ((FPS > 5) ? 5 : ((FPS > 1) ? 1 : 0));
	}

	game->update(dt);
}

void draw() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	game->draw();
}

int main(int argc, char** argv) {
	GLFWmanager glfw(800, 600);
	GLEWmanager glew;

	if (DEBUG)
		initDebug();
	init();
	CHECK_GL_ERR;
	while (!glfwWindowShouldClose(Window::window)) {
		update();
		draw();
		CHECK_GL_ERR;
		glfwSwapBuffers(Window::window);
		glfwPollEvents();
	}

	return 0;
}