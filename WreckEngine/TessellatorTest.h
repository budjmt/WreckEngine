#pragma once

#include "Game.h"

class TessellatorTest : public Game {
public:
    TessellatorTest();
    void update(double dt) override;
    void postUpdate() override;
    void draw() override;
private:
    struct RenderData {
        GLprogram prog;
        GLuniform<mat4> mat;
        GLuniform<vec3> pos;
    };
    RenderData planetData, controlData;
};