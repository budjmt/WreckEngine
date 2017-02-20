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
        GLprogram material, program;
        GLtexture cubemap;
        GLuniform<mat4> viewProjection;
        GLuniform<float> compTime;
        GLuniform<float> compZoom;
        GLresource<float> tessLevelInner, tessLevelOuter, radius;
    };

    RenderData renderData;
    shared<Camera> camera;
    shared<Entity> cube;
    float time = 0.0f;
    float direction = -1.0f;
    bool rotate = true;
};
