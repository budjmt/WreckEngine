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

std::map<const char*, GLuint> Drawable::loadedTextures;

GLuint Drawable::genTexture(const char* texFile) {
	//check if the image was already loaded
	if (loadedTextures.find(texFile) != loadedTextures.end()) {
		return loadedTextures[texFile];
	}
	FIBITMAP* bitmap = FreeImage_Load(FreeImage_GetFileType(texFile), texFile);
	//we convert the 24bit bitmap to 32bits
	FIBITMAP* image = FreeImage_ConvertTo32Bits(bitmap);
	//delete the 24bit bitmap from memory
	FreeImage_Unload(bitmap);
	int w = FreeImage_GetWidth(image);
	int h = FreeImage_GetHeight(image);
	GLubyte* textureData = FreeImage_GetBits(image);

	GLuint texture = 0;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	//the texture is loaded in BGRA format
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, (GLvoid*)textureData);
	FreeImage_Unload(image);

	loadedTextures[texFile] = texture;
	return texture;
}