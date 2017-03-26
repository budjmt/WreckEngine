#pragma once

#include "Game.h"

class CubemapTest : public Game
{
public:
    CubemapTest();
    void update(double dt) override;
    void postUpdate() override;
    void draw() override;
private:
    struct RenderData {
        GLprogram material;
        GLuniform<mat4> viewProjection;
        GLresource<float> tessLevelInner, tessLevelOuter, radius;
    };
    struct NoiseMapData {
        GLtexture tex;
        GLprogram prog;
        GLresource<float, true> zoom;
    };
    struct NormalMapData {
        GLtexture tex;
        GLprogram prog;
        GLresource<vec3, true> camPos;
        GLresource<float> radius;
    };

    RenderData renderData;
    NoiseMapData noiseMap;
    NormalMapData normalMap;
    shared<Camera> camera;
    shared<Entity> cube;
    float time = 0.0f;
    float direction = -1.0f;
    bool rotate = true;
};
