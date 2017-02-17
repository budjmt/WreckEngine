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
        GLresource<float> tessLevelInner, tessLevelOuter, radius;
    };

    void calcCameraPosition();

    RenderData renderData;
    shared<Camera> camera;
    float time = 0.0f;
    float direction = 1.0f;
    bool rotate = false;
};
