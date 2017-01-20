#include "GL/glew.h"

#include "ShaderHelper.h"
#include <iostream>
#include <vector>

#include "File.h"
#include "HotSwap.h"

using namespace std;

#define LOAD_HOTSWAP_PARAMS(file, type) loadShader(file, type), file, type

GLshader loadShader(const char* file, GLenum shaderType) {
    return File::load<File::Extension::GLSL>(file, shaderType);
}

GLprogram loadProgram(const char* vertexFile, const char* fragmentFile) {
	
    auto shaderProg = HotSwap::Shader::create();
	shaderProg->vertex.set(LOAD_HOTSWAP_PARAMS(vertexFile, GL_VERTEX_SHADER));
	if (!shaderProg->vertex.get().valid()) {
		cout << "Error: Vertex shader from " << vertexFile << " could not be used." << endl;
		return GLprogram();
	}
	shaderProg->fragment.set(LOAD_HOTSWAP_PARAMS(fragmentFile, GL_FRAGMENT_SHADER));
	if (!shaderProg->fragment.get().valid()) {
		cout << "Error: Fragment shader from " << fragmentFile << " could not be used." << endl;
		return GLprogram();
	}
	//cout << "Files read successfully." << endl;
    shaderProg->setupProgram();
    auto prog = shaderProg->getProgram();

	if (prog.getVal(GL_LINK_STATUS) == GL_TRUE) {
		cout << "Successfully loaded " << vertexFile << " and " << fragmentFile << endl;
		return prog;
	}

	auto logLength = prog.getVal(GL_INFO_LOG_LENGTH);
	auto log = std::vector<char>(logLength);
	glGetProgramInfoLog(prog(), logLength, 0, &log[0]);
	cout << "PROGRAM LINK ERROR: " << vertexFile << " + " << fragmentFile << ": " << &log[0] << endl;
	return GLprogram();
}