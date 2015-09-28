#pragma once
#include "GL\glew.h"
#include "glm\glm.hpp"

char* loadTextFile(const char* file);
GLuint loadShader(const char* file, GLenum shaderType);
GLuint loadShaderProgram(const char* vertexFile, const char* fragmentFile);
void setShaderColor(GLuint prog, const char* varName, float r, float g, float b);

