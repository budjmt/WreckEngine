#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"

#include "GLDebugMessageCallback.h"
#include "ShaderHelper.h"
#include "ModelHelper.h"
#include "Mouse.h"
#include "TriPlay.h"

using namespace std;

GLFWwindow* window;
GLuint shaderProg;

double prevFrame;
Mouse* mouse;

TriPlay* game;

void APIENTRY GLDebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *msg, const void *data);

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

void mouse_move_callback(GLFWwindow* window, double x, double y);

void init() {
	shaderProg = loadShaderProgram("Shaders/matvertexShader.glsl","Shaders/matfragmentShader.glsl");
	if(shaderProg != 0) {
		glUseProgram(shaderProg);
		setShaderColor(shaderProg,"tint",1,1,1);

		//game = new TriPlay(shaderProg);
	}

	mouse = new Mouse();
	mouse_move_callback(window,0,0);//this is cheating but it works

	game = new TriPlay(shaderProg, window);
}

void initGraphics(GLFWwindow* window) {
	int width, height;
	glfwGetWindowSize(window, &width, &height);

	float aspect = (float)width / (float)height;
	glViewport(0, 0, width, height);

	glEnable(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	//glDebugMessageCallback(GLDebugMessageCallback, NULL);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//cornflower blue
	glClearColor(0.392f, 0.584f, 0.929f, 1.0f);
	//glClearColor(0.f, 0.f, 0.f, 1.f);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);//render as wireframe

	/*glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, width, 0, height);
	glMatrixMode(GL_MODELVIEW);*/
}

void update() {
	double currFrame = glfwGetTime();
	double dt = currFrame - prevFrame;
	prevFrame = currFrame;

	mouse->prevx = glm::mix(mouse->prevx, mouse->x, 0.15f);
	mouse->prevy = glm::mix(mouse->prevy, mouse->y, 0.15f);
	
	game->update(window, mouse, prevFrame, dt);
}

void draw() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	game->draw();

	glFlush();
}

int main(int argc, char** argv) {
	if (!glfwInit())
		return -1;

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

void APIENTRY GLDebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *msg, const void *data) {
	char* _source;
	char* _type;
	char* _severity;

	switch (source) {
	case GL_DEBUG_SOURCE_API:
		_source = "API";
		break;

	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		_source = "WINDOW SYSTEM";
		break;

	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		_source = "SHADER COMPILER";
		break;

	case GL_DEBUG_SOURCE_THIRD_PARTY:
		_source = "THIRD PARTY";
		break;

	case GL_DEBUG_SOURCE_APPLICATION:
		_source = "APPLICATION";
		break;

	case GL_DEBUG_SOURCE_OTHER:
		_source = "UNKNOWN";
		break;

	default:
		_source = "UNKNOWN";
		break;
	}

	switch (type) {
	case GL_DEBUG_TYPE_ERROR:
		_type = "ERROR";
		break;

	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		_type = "DEPRECATED BEHAVIOR";
		break;

	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		_type = "UDEFINED BEHAVIOR";
		break;

	case GL_DEBUG_TYPE_PORTABILITY:
		_type = "PORTABILITY";
		break;

	case GL_DEBUG_TYPE_PERFORMANCE:
		_type = "PERFORMANCE";
		break;

	case GL_DEBUG_TYPE_OTHER:
		_type = "OTHER";
		break;

	case GL_DEBUG_TYPE_MARKER:
		_type = "MARKER";
		break;

	default:
		_type = "UNKNOWN";
		break;
	}

	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH:
		_severity = "HIGH";
		break;

	case GL_DEBUG_SEVERITY_MEDIUM:
		_severity = "MEDIUM";
		break;

	case GL_DEBUG_SEVERITY_LOW:
		_severity = "LOW";
		break;

	case GL_DEBUG_SEVERITY_NOTIFICATION:
		_severity = "NOTIFICATION";
		break;

	default:
		_severity = "UNKNOWN";
		break;
	}

	printf("%d: %s of %s severity, raised from %s: %s\n",
		id, _type, _severity, _source, msg);
}