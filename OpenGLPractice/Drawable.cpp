#include "Drawable.h"

#include "glm/gtx/transform.hpp"

Drawable::Drawable()
	: shaderProg(dshaderProg), colorLoc(dcolorLoc)
{
}


Drawable::Drawable(const Drawable& other) 
	: shaderProg(dshaderProg), colorLoc(dcolorLoc)
{
	shaderProg = other.shaderProg;
	colorLoc = other.colorLoc;
}

Drawable::~Drawable()
{
	glDeleteVertexArrays(1, &vArray);
	glDeleteBuffers(1, &vBuffer);
}

void Drawable::draw(GLfloat x, GLfloat y, GLfloat xScale, GLfloat yScale) {
	glm::vec3 pos = glm::vec3(x, y, 0);
	glm::vec3 scale = glm::vec3(xScale, yScale, 1);
	glm::vec3 rotAxis = glm::vec3(0, 0, 1);
	float rot = 0;
	setWorldMatrix(pos, scale, rotAxis, rot);

	//now the stuff done in init happens automatically since they were done
	//while it was active
	glBindVertexArray(vArray);

	//actual draw call is reserved for children
}

void Drawable::draw(glm::vec3 pos, glm::vec3 scale, glm::vec3 rotAxis, float rot) {
	setWorldMatrix(pos, scale, rotAxis, rot);

	//now the stuff done in init happens automatically since they were done
	//while it was active
	glBindVertexArray(vArray);
}

void Drawable::draw(Transform t) {
	draw(t.position, t.scale, t.rotAxis, t.rotAngle);
}

void Drawable::setWorldMatrix(glm::vec3 pos, glm::vec3 scaleV, glm::vec3 rotAxis, float rot) {
	glm::mat4 translate = glm::translate(pos);
	glm::mat4 scale = glm::scale(scaleV);
	glm::mat4 rotate = glm::rotate(rot, rotAxis);
	glUniformMatrix4fv(worldMatrix, 1, GL_FALSE, &(translate * scale * rotate)[0][0]);
}
