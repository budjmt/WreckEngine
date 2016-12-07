#pragma once

#include "GL/glew.h"
#include "glm/glm.hpp"
#include "FreeImage.h"

#include "gl_structs.h"

#include "Transform.h"

#include <unordered_map>

#include "Render.h"

constexpr size_t FLOATS_PER_VERT = 3;
constexpr size_t FLOATS_PER_NORM = 3;
constexpr size_t FLOATS_PER_UV = 2;

class Drawable {
public:
    Render::MaterialPass* renderer;
    Render::Info material;
    vec4& color = _color.value;

    void draw(GLfloat x, GLfloat y, GLfloat xScale, GLfloat yScale);
    void draw(vec3 pos, vec3 scale, vec3 rotAxis, float rot);
    void draw(const mat4& translate, const mat4& rotate, const mat4& scale);
    void draw(Transform* t);
    virtual void draw(const mat4& world);
    void setWorldMatrix(const mat4& world);

    static GLtexture genTexture2D(const char* texFile);
    static void unloadTextures();
protected:
    GLVAO vArray;
    GLresource<vec4> _color;
    GLresource<mat4> worldMatrix, iTworldMatrix;

    static std::unordered_map<const char*, GLtexture> loadedTextures; // all currently loaded textures
};