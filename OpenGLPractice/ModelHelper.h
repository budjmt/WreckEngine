#pragma once
#include "GL\glew.h"
#include "glm\glm.hpp"
#include "Mesh.h"
#include <string>
#include <vector>

//char* loadFBX(const char* file);
Mesh* loadOBJ(const char* file, char* texture, GLuint shader);//loads a .obj

std::vector<std::string> tokenize(std::string str, std::string delimiter);
