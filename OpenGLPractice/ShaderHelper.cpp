#include "GL/glew.h"

#include "ShaderHelper.h"
#include <iostream>
#include <fstream>

using namespace std;

char* loadTextFile(const char* file) {
	ifstream infile;
	infile.open(file,ios::binary);
	if(infile.is_open()) {
		infile.seekg(0,ios::end);
		int length = (int)infile.tellg();
		infile.seekg(0,ios::beg);

		char* filecontents = new char[length + 1];
		infile.read(filecontents, length);
		filecontents[length] = 0;
		infile.close();
		return filecontents;
	}
	return 0;
}

GLuint loadShader(const char* file, GLenum shaderType) {
	const char* filecontents = loadTextFile(file);
	if(filecontents == 0) {
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

void setShaderColor(GLuint prog, const char* varName, float r, float g, float b) {
	GLint color = glGetUniformLocation(prog, varName);
	glUniform4f(color,r,g,b,1.0f);
}