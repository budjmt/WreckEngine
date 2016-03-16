#pragma once

#include "GL/glew.h"
#include "glm/glm.hpp"
#include "FreeImage.h"

#include "Transform.h"

#include <map>

const GLint FLOATS_PER_VERT = 3;
const GLint FLOATS_PER_NORM = 3;
const GLint FLOATS_PER_UV = 2;

class Drawable
{
public:
	Drawable();
	Drawable(const Drawable& other);
	virtual ~Drawable();
	GLuint& shaderProg;
	GLint& colorLoc;
	virtual void draw(GLfloat x, GLfloat y, GLfloat xScale, GLfloat yScale);
	virtual void draw(glm::vec3 pos, glm::vec3 scale, glm::vec3 rotAxis, float rot);
	virtual void draw(Transform t);
	void setWorldMatrix(glm::vec3 pos, glm::vec3 scale, glm::vec3 rotAxis, float rot);
	GLuint genTexture(const char* texFile);
protected:
	GLuint vBuffer;
	GLuint vArray;
	GLuint dshaderProg;
	GLint offset, scale, worldMatrix, iTworldMatrix, dcolorLoc;
	static std::map<const char*, GLuint> loadedTextures;//all currently loaded textures
};