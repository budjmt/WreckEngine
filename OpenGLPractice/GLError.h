#pragma once
#include "GL/glew.h"
#include "glfw/glfw3.h"

#include <stdio.h>

static void GLPrintError(GLenum error) {
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
}
