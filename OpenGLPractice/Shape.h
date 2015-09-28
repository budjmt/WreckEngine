#pragma once

#include "GL/glew.h"
#include "glm/glm.hpp"

#include "ShaderHelper.h"
#include "Drawable.h"

class Shape : public Drawable
{
public:
	Shape(GLfloat* verts, GLint numV, GLuint shader);
	void draw(GLfloat x, GLfloat y, GLfloat xScale, GLfloat yScale);
	void draw(glm::vec3 pos, glm::vec3 scale, glm::vec3 rotAxis, float rot);
private:
	GLuint numVerts;
};

