#pragma once

#include "GL/glew.h"
#include "glm/glm.hpp"
#include "FreeImage.h"

#include "gl_structs.h"

#include "Transform.h"

#include <map>

const size_t FLOATS_PER_VERT = 3;
const size_t FLOATS_PER_NORM = 3;
const size_t FLOATS_PER_UV = 2;

class Drawable
{
public:
	GLprogram& shaderProg = _shaderProg;
	GLuniform<vec4>& colorLoc = _colorLoc;
	virtual void draw(GLfloat x, GLfloat y, GLfloat xScale, GLfloat yScale);
	virtual void draw(vec3 pos, vec3 scale, vec3 rotAxis, float rot);
	virtual void draw(Transform* t);
	void setWorldMatrix(vec3 pos, vec3 scale, vec3 rotAxis, float rot);
	GLuint genTexture(const char* texFile);
protected:
	GLVAO vArray;
	GLprogram _shaderProg;
	GLuniform<vec4> _colorLoc;
	GLuniform<mat4> worldMatrix, iTworldMatrix;
	static std::map<const char*, GLuint> loadedTextures;//all currently loaded textures
};