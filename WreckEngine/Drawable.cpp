#include "Drawable.h"

#include "glm/gtx/transform.hpp"

void Drawable::draw(Transform* t) {
    draw(t->getMats()->world);
}
void Drawable::draw(GLfloat x, GLfloat y, GLfloat xScale, GLfloat yScale) {
    draw(vec3(x, y, 0), vec3(xScale, yScale, 1), vec3(0, 0, 1), 0);
}
void Drawable::draw(vec3 pos, vec3 scale, vec3 rotAxis, float rot) {
    draw(glm::translate(pos), glm::rotate(rot, rotAxis), glm::scale(scale));
}
void Drawable::draw(const mat4& translate, const mat4& rotate, const mat4& scale) {
    draw(translate * rotate * scale);
}
void Drawable::draw(const mat4& world) {
    setWorldMatrix(world);

    //actual draw call is reserved for children
}

void Drawable::setWorldMatrix(const mat4& world) {
    worldMatrix.value = world;
    iTworldMatrix.value = inv_tp_tf(world);
}

std::unordered_map<const char*, GLtexture> Drawable::loadedTextures;

void Drawable::unloadTextures() {
    for (auto& texture : loadedTextures) {
        texture.second.unload();
    }
    loadedTextures.clear();
}

#include "File.h"

GLtexture Drawable::genTexture2D(const char* texFile) {
    //check if the image was already loaded
    if (loadedTextures.find(texFile) != loadedTextures.end()) {
        return loadedTextures[texFile];
    }
    
    GLtexture texture;
    auto image = File::load<File::Extension::PNG>(texFile);
    if (!image.image) return texture;
    
    texture.create(GL_TEXTURE_2D, 0);
    texture.bind();
    //the texture is loaded in BGRA format
    texture.set2D<GLubyte>(image.bytes, image.width, image.height, GL_BGRA);
    texture.genMipMap();

    loadedTextures[texFile] = texture;
    return texture;
}