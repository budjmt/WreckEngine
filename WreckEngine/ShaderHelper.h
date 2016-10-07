#pragma once
#include "GL\glew.h"
#include "glm\glm.hpp"

#include "gl_structs.h"

const char* loadTextFile(const char* file);
GLshader loadShader(const char* file, GLenum shaderType);
GLprogram loadProgram(const char* vertexFile, const char* fragmentFile);

