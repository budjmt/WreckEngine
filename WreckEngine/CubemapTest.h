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
        GLprogram program;
        GLtexture cubemap;
        GLresource<mat4> viewProjection;
        GLresource<mat4> model;
        GLresource<mat3> normalMatrix;
        GLresource<float> tessLevelInner;
        GLresource<float> tessLevelOuter;
        GLresource<float> radius;
    };

    void calcCameraPosition();

    RenderData renderData;
    shared<Camera> camera;
    float time = 0.0f;
    float direction = 1.0f;
    bool rotate = false;
};
