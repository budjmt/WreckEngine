#pragma once

#include "gl_structs.h"
#include "Entity.h"
#include "MarchMath.h"

class ComputeEntity : public Entity
{
public:
    GLprogram program;
    std::function<void()> update_uniforms;
    glm::ivec3 dispatchSize;
    // the compute shader will be run every "updateFreq" seconds
    // default is 16x / second
    float updateFreq = 1.0f / 16.0f;
    bool synchronize = true;

    void update(double dt) override;
    void draw() override;

private:
    float timeSinceUpdate = 0.0f;
};

class ComputeTextureEntity : public ComputeEntity
{
public:
    GLtexture texture;
    GLenum access = GL_WRITE_ONLY;
    GLenum format = GL_RGBA32F;
    GLint index = 0;
    // TODO - level, layer

    void draw() override;
};
