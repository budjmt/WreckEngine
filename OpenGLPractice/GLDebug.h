#ifndef GL_DEBUG
#define GL_DEBUG

#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include <stdio.h>

void APIENTRY GLDebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const GLvoid *userParam);
void APIENTRY GLDebugMessageCallbackAMD(GLuint id, GLenum category, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam);
void GLPrintDebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *msg, const GLvoid *userParam);
void initDebug();

/*
// =============== INIT DEBUG OUTPUT ================
// The following function calls should be made directly after OpenGL
// initialization.

// Enable the debugging layer of OpenGL
//
// GL_DEBUG_OUTPUT - Faster version but not useful for breakpoints
// GL_DEBUG_OUTPUT_SYNCHRONUS - Callback is in sync with errors, so a breakpoint
// can be placed on the callback in order to get a stacktrace for the GL error.

glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

// Set the function that will be triggered by the callback, the second parameter
// is the data parameter of the callback, it can be useful for different
// contexts but isn't necessary for our simple use case.
glDebugMessageCallback(GLDebugMessageCallback, NULL);

*/

#endif