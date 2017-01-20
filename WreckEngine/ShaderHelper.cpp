#include "GL/glew.h"

#include "ShaderHelper.h"
#include <iostream>
#include <vector>

#include "File.h"

using namespace std;

GLshader loadShader(const char* file, GLenum shaderType) {
    return File::load<File::Extension::GLSL>(file, shaderType);
}

GLprogram loadProgram(const char* vertexFile, const char* fragmentFile) {
	
	GLprogram shaderProg;
	shaderProg.vertex = loadShader(vertexFile, GL_VERTEX_SHADER);
	if (!shaderProg.vertex.valid()) {
		cout << "Error: Vertex shader from " << vertexFile << " could not be used." << endl;
		return GLprogram();
	}
	shaderProg.fragment = loadShader(fragmentFile, GL_FRAGMENT_SHADER);
	if (!shaderProg.fragment.valid()) {
		cout << "Error: Fragment shader from " << fragmentFile << " could not be used." << endl;
		return GLprogram();
	}
	//cout << "Files read successfully." << endl;
	shaderProg.create();
	shaderProg.link();

	if (shaderProg.getVal(GL_LINK_STATUS) == GL_TRUE) {
		cout << "Successfully loaded " << vertexFile << " and " << fragmentFile << endl;
		return shaderProg;
	}

	auto logLength = shaderProg.getVal(GL_INFO_LOG_LENGTH);
	auto log = std::vector<char>(logLength);
	glGetProgramInfoLog(shaderProg(), logLength, 0, &log[0]);
	cout << "PROGRAM LINK ERROR: " << vertexFile << " + " << fragmentFile << ": " << &log[0] << endl;
	return GLprogram();
}