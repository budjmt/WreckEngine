#ifndef P_GL_ERROR
#define P_GL_ERROR

#include "GL/glew.h"
#include "glfw/glfw3.h"

#include <stdio.h>

#define BREAK "----------------------------------------------\n"

#define STRINGIZE2(x) #x
#define STRINGIZE(x) STRINGIZE2(x)

#define CLEAR_GL_ERR glGetError()
#define CHECK_GL_ERR GLCheckError("GL-ERROR; \"" __FILE__ "\" before line " STRINGIZE(__LINE__) ": \n")

static void GLPrintError(GLenum error, const char* prepend = "") {
	printf(BREAK "%s", prepend);
	switch (error) {
	case GL_INVALID_ENUM:
		printf("Invalid enum value\n");
		break;
	case GL_INVALID_VALUE:
		printf("Numeric argument out of range\n");
		break;
	case GL_INVALID_OPERATION:
		printf("Operation not allowed in current state\n");
		break;
	case GL_INVALID_FRAMEBUFFER_OPERATION:
		printf("Incomplete frame buffer object\n");
		break;
	case GL_OUT_OF_MEMORY:
		printf("Insufficient memory for command\n");
		break;
	case GL_STACK_UNDERFLOW:
		printf("Stack underflow\n");
		break;
	case GL_STACK_OVERFLOW:
		printf("Stack overflow\n");
		break;
	default:
		printf("Unknown error\n");
		break;
	}
	printf(BREAK);
}

static inline void GLCheckError(const char* prepend = "") {
	auto err = glGetError();
	if (err) GLPrintError(err, prepend);
}

#endif