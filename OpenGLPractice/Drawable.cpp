#include "Drawable.h"

#include "glm/gtx/transform.hpp"

void Drawable::draw(Transform* t) { draw(t->position, t->scale, t->rotAxis(), t->rotAngle()); }
void Drawable::draw(GLfloat x, GLfloat y, GLfloat xScale, GLfloat yScale) { draw(vec3(x, y, 0), vec3(xScale, yScale, 1), vec3(0, 0, 1), 0); }
void Drawable::draw(vec3 pos, vec3 scale, vec3 rotAxis, float rot) {
	setWorldMatrix(pos, scale, rotAxis, rot);

	//now the stuff done in init happens automatically since they were done
	//while it was active
	vArray.bind();

	//actual draw call is reserved for children
}

void Drawable::setWorldMatrix(vec3 pos, vec3 scaleV, vec3 rotAxis, float rot) {
	auto translate = glm::translate(pos);
	auto scale = glm::scale(scaleV);
	auto rotate = glm::rotate(rot, rotAxis);
	auto world = translate * rotate * scale;
	worldMatrix.update(world);
	iTworldMatrix.update(glm::inverse(glm::transpose(world)));
}

std::map<const char*, GLuint> Drawable::loadedTextures;

GLuint Drawable::genTexture(const char* texFile) {
	//check if the image was already loaded
	if (loadedTextures.find(texFile) != loadedTextures.end()) {
		return loadedTextures[texFile];
	}
	auto bitmap = FreeImage_Load(FreeImage_GetFileType(texFile), texFile);
	//we convert the 24bit bitmap to 32bits
	auto image = FreeImage_ConvertTo32Bits(bitmap);
	//delete the 24bit bitmap from memory
	FreeImage_Unload(bitmap);
	auto w = FreeImage_GetWidth(image);
	auto h = FreeImage_GetHeight(image);
	auto textureData = FreeImage_GetBits(image);

	GLuint texture = 0;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	//the texture is loaded in BGRA format
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, (GLvoid*)textureData);
	FreeImage_Unload(image);

	loadedTextures[texFile] = texture;
	return texture;
}