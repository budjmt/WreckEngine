#include "GLDebug.h"

// Callback function for printing debug statements
void APIENTRY GLDebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const GLvoid *userParam) {
	GLPrintDebugMessageCallback(source, type, id, severity, length, message, userParam);
}

void APIENTRY GLDebugMessageCallbackAMD(GLuint id, GLenum category, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam) {
	GLPrintDebugMessageCallback(category, category, id, severity, length, message, userParam);
}


// This is free and unencumbered software released into the public domain.
// 
// Anyone is free to copy, modify, publish, use, compile, sell, or distribute
// this software, either in source code form or as a compiled binary, for any
// purpose, commercial or non-commercial, and by any means.
// 
// In jurisdictions that recognize copyright laws, the author or authors of this
// software dedicate any and all copyright interest in the software to the
// public domain. We make this dedication for the benefit of the public at large
// and to the detriment of our heirs and successors. We intend this dedication
// to be an overt act of relinquishment in perpetuity of all present and future
// rights to this software under copyright law.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// 
// For more information, please refer to <http://unlicense.org/>

// REQUIREMENTS: OpenGL version with the KHR_debug extension available.

//Minor changes made by Michael Cohen 2015
void GLPrintDebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *msg, const GLvoid *userParam) {
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
		_source = "OTHER";
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

/*
Debug callbacks only became supported in OpenGL 4.3, which added GL_DEBUG_OUTPUT_SYNCHRONOUS to the spec
Prior to this, there was only the ARB support on specific graphics cards
This code allows for the various possible permutations, including AMD cards, which don't support synchronous debug message callbacks
*/
void initDebug() {
	printf("NOTICE: This application is in DEBUG mode. This allows for OpenGL debug message callbacks, as well as the ability to draw debug primitives through the DrawDebug singleton. ");
	printf("This may impact the speed and performance of this application, so if this is a final build, it is recommended this mode be turned off. ");
	printf("To do so, go to the DrawDebug.h file and change the value of the DEBUG constant to false.");
    if (glewIsSupported("GL_VERSION_4_3")) {
    CORE:
        //works in OpenGL 4.3 and higher
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(GLDebugMessageCallback, NULL);
        glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, 0, GL_FALSE);
    }
	else if (glfwExtensionSupported("KHR_debug") == GL_TRUE) {
		if (glfwExtensionSupported("glDebugMessageCallback") == GL_TRUE) {
            goto CORE;
		}
		else if (glfwExtensionSupported("glDebugMessageCallbackARB") == GL_TRUE) {
			//in the event that native debugging is unsupported, fall back on ARB debugging
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
			glDebugMessageCallbackARB(GLDebugMessageCallback, NULL);
		}
		else if (glfwExtensionSupported("glDebugMessageCallbackAMD") == GL_TRUE) {
			//no synchronous debugging available, also use the AMD format
			glDebugMessageCallbackAMD(GLDebugMessageCallbackAMD, NULL);
		}
		else {
			printf("What the heck is going on with your graphics card...\n\n");
		}
	}
	else {
		printf("WARNING: debug message callbacks unsupported in this version of OpenGL (below 4.3) or on this graphics card (may support 4.0 and after); recommend turning off DEBUG flag\n\n");
	}
}