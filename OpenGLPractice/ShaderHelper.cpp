#include "GL/glew.h"

#include "ShaderHelper.h"
#include <iostream>
#include <fstream>
#include <vector>

using std::ios;
using std::cout; using std::endl;

const char* loadTextFile(const char* file) {
	std::ifstream infile;
	infile.open(file, ios::binary);
	if(infile.is_open()) {
		infile.seekg(0, ios::end);
		int length = (int)infile.tellg();
		infile.seekg(0, ios::beg);

		char* filecontents = new char[length + 1];
		infile.read(filecontents, length);
		filecontents[length] = 0;
		infile.close();
		return filecontents;
	}
	return nullptr;
}

GLuint loadShader(const char* file, GLenum shaderType) {
	const char* filecontents = loadTextFile(file);
	if(filecontents == nullptr) {
		cout << "Error! File " << file << " could not be read." << endl;
		return 0;
	}
	
	GLuint shader = glCreateShader(shaderType);
	glShaderSource(shader,1,&filecontents,0);
	glCompileShader(shader);
	delete[] filecontents;

	GLint result;
	glGetShaderiv(shader,GL_COMPILE_STATUS,&result);
	if(result == GL_TRUE)
		return shader;

	GLint logLength;
	glGetShaderiv(shader,GL_INFO_LOG_LENGTH,&logLength);
	char* log = new char[logLength];
	glGetShaderInfoLog(shader,logLength,0,log);
	cout << log << endl;
	glDeleteShader(shader);
	delete[] log;
	return 0;
}

GLshader loadGLShader(const char* file, GLenum shaderType) {
	auto fileContents = unique<const char>(loadTextFile(file));
	if (!fileContents) {
		cout << "Error! File " << file << " could not be read." << endl;
		return GLshader();
	}

	GLshader shader;
	shader.create(fileContents.get(), shaderType);

	GLint result;
	glGetShaderiv(shader(), GL_COMPILE_STATUS, &result);
	if (result == GL_TRUE)
		return shader;

	GLint logLength;
	glGetShaderiv(shader(), GL_INFO_LOG_LENGTH, &logLength);
	auto log = std::vector<char>(logLength);
	glGetShaderInfoLog(shader(), logLength, 0, &log[0]);
	cout << &log[0] << endl;
	return GLshader();
}

GLuint loadShaderProgram(const char* vertexFile, const char* fragmentFile) {
	GLuint vShader = loadShader(vertexFile,GL_VERTEX_SHADER);
	if(vShader == 0) {
		cout << "Error: Vertex shader from " << vertexFile << " could not be used." << endl;
		return 0;
	}
	GLuint fShader = loadShader(fragmentFile,GL_FRAGMENT_SHADER);
	if(fShader == 0) {
		cout << "Error: Fragment shader from " << fragmentFile << " could not be used." << endl;
		return 0;
	}
	//cout << "Files read successfully." << endl;
	GLuint shaderProg = glCreateProgram();
	glAttachShader(shaderProg,vShader);
	glAttachShader(shaderProg,fShader);
	glLinkProgram(shaderProg);

	GLint linkStatus;
	glGetProgramiv(shaderProg,GL_LINK_STATUS,&linkStatus);
	if(linkStatus == GL_TRUE) {
		cout << "Successfully loaded " << vertexFile << " and " << fragmentFile << endl;
		return shaderProg;
	}

	GLint logLength;
	glGetProgramiv(shaderProg,GL_INFO_LOG_LENGTH,&logLength);
	char* log = new char[logLength];
	glGetProgramInfoLog(shaderProg,logLength,0,log);
	cout << log << endl;
	glDeleteProgram(shaderProg);
	delete[] log;
	return 0;
}

GLprogram loadGLProgram(const char* vertexFile, const char* fragmentFile) {
	
	GLprogram shaderProg;
	shaderProg.vertex = loadGLShader(vertexFile, GL_VERTEX_SHADER);
	if (!shaderProg.vertex.valid()) {
		cout << "Error: Vertex shader from " << vertexFile << " could not be used." << endl;
		return GLprogram();
	}
	shaderProg.fragment = loadGLShader(fragmentFile, GL_FRAGMENT_SHADER);
	if (!shaderProg.fragment.valid()) {
		cout << "Error: Fragment shader from " << fragmentFile << " could not be used." << endl;
		return GLprogram();
	}
	//cout << "Files read successfully." << endl;
	shaderProg.create();
	shaderProg.link();

	GLint linkStatus;
	glGetProgramiv(shaderProg(), GL_LINK_STATUS, &linkStatus);
	if (linkStatus == GL_TRUE) {
		cout << "Successfully loaded " << vertexFile << " and " << fragmentFile << endl;
		return shaderProg;
	}

	GLint logLength;
	glGetProgramiv(shaderProg(), GL_INFO_LOG_LENGTH, &logLength);
	auto log = std::vector<char>(logLength);
	glGetProgramInfoLog(shaderProg(), logLength, 0, &log[0]);
	cout << &log[0] << endl;
	return GLprogram();
}

void setShaderColor(GLuint prog, const char* varName, float r, float g, float b) {
	GLint color = glGetUniformLocation(prog, varName);
	glUniform4f(color,r,g,b,1.0f);
}