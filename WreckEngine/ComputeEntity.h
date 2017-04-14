#pragma once

#include "gl_structs.h"
#include "Entity.h"
#include "MarchMath.h"

class ComputeEntity : public Entity {
public:
    using Entity::Entity;

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

class ComputeTextureEntity : public ComputeEntity {
public:
    using ComputeEntity::ComputeEntity;

    inline void addImage(GLtexture tex, GLenum access = GL_WRITE_ONLY, GLenum format = GL_RGBA32F) {
        auto data = ImageData{ tex, access, format /* TODO - level, layer */ };
        images.push_back(data);
    }

    void draw() override;

private:
    struct ImageData {
        GLtexture tex;
        GLenum access;
        GLenum format;
        // TODO - level, layer

        inline bool layered() const {
            return tex.target == GL_TEXTURE_CUBE_MAP || tex.target == GL_TEXTURE_CUBE_MAP_ARRAY;
        }
    };

    std::vector<ImageData> images;
};
