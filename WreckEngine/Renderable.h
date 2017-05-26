#pragma once

#include "gl_structs.h"
#include "GraphicsWorker.h"

#include <unordered_map>

constexpr size_t FLOATS_PER_VERT = 3;
constexpr size_t FLOATS_PER_NORM = 3;
constexpr size_t FLOATS_PER_UV = 2;

// Anything that is rendered to the screen; goes through a renderer
class Renderable : public GraphicsWorker {
public:
    Render::MaterialPass* renderer;
    vec4& color = _color.value;

    void draw(Transform* t, Entity* entity) override;
    virtual void draw(const mat4& world, Entity* entity);
    void setWorldMatrix(const mat4& world);

    static GLtexture genTexture2D(const char* texFile);
    static void unloadTextures();
protected:
    GLVAO vArray;
    GLresource<vec4> _color;
    GLresource<mat4> worldMatrix, iTworldMatrix;

    static std::unordered_map<const char*, GLtexture> loadedTextures; // all currently loaded textures
};