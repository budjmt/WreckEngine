#include "Drawable.h"

#include "glm/gtx/transform.hpp"

void Drawable::draw(Transform* t) { draw(t->getMats()->world); }
void Drawable::draw(GLfloat x, GLfloat y, GLfloat xScale, GLfloat yScale) { draw(vec3(x, y, 0), vec3(xScale, yScale, 1), vec3(0, 0, 1), 0); }
void Drawable::draw(vec3 pos, vec3 scale, vec3 rotAxis, float rot) { draw(glm::translate(pos), glm::rotate(rot, rotAxis), glm::scale(scale)); }
void Drawable::draw(const mat4& translate, const mat4& rotate, const mat4& scale) { draw(translate * rotate * scale); }
void Drawable::draw(const mat4& world) {
	setWorldMatrix(world);

	//now the stuff done in init happens automatically since they were done
	//while it was active
	vArray.bind();

	//actual draw call is reserved for children
}

void Drawable::setWorldMatrix(const mat4& world) {
	worldMatrix.update(world);
	iTworldMatrix.update(glm::inverse(glm::transpose(world)));
}

std::unordered_map<const char*, GLtexture> Drawable::loadedTextures;

GLtexture Drawable::genTexture2D(const char* texFile) {
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

	GLtexture texture;
	texture.create();
	texture.bind();
	//the texture is loaded in BGRA format
	texture.set2D<GLubyte>(textureData, w, h, GL_BGRA);
	FreeImage_Unload(image);

	loadedTextures[texFile] = texture;
	return texture;
}