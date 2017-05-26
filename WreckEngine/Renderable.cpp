#include "Renderable.h"

#include "glm/gtx/transform.hpp"

void Renderable::draw(Transform* t, Entity* entity) {
    draw(t->getMats()->world, entity);
}

void Renderable::draw(const mat4& world, Entity* entity) {
    setWorldMatrix(world);

    //actual draw call is reserved for children
}

void Renderable::setWorldMatrix(const mat4& world) {
    worldMatrix.value = world;
    iTworldMatrix.value = inv_tp_tf(world);
}

std::unordered_map<const char*, GLtexture> Renderable::loadedTextures;

void Renderable::unloadTextures() {
    for (auto& texture : loadedTextures) {
        texture.second.unload();
    }
    loadedTextures.clear();
}

#include "File.h"

GLtexture Renderable::genTexture2D(const char* texFile) {
    //check if the image was already loaded
    if (loadedTextures.find(texFile) != loadedTextures.end()) {
        return loadedTextures[texFile];
    }

    GLtexture texture;
    auto image = File::load<File::Extension::PNG>(texFile);
    if (!image) return texture;

    texture.create(GL_TEXTURE_2D, 0);
    texture.bind();
#if WR_USE_FREEIMAGE
    //the texture is loaded in BGRA format
    texture.set2D<GLubyte>(image.bytes, image.width, image.height, GL_BGRA);
#else
    texture.set2D<GLubyte>(image.bytes.get(), image.width, image.height, GL_RGBA);
#endif
    texture.genMipMap();

#if WR_USE_FREEIMAGE
    image.unload();
#endif

    loadedTextures[texFile] = texture;
    return texture;
}