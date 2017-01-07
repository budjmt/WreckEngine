#pragma once
#include "Game.h"

#include "glm/gtc/random.hpp"
#include "glm/gtc/constants.hpp"

#include "External.h"
#include "Text.h"
#include "Camera.h"
#include "ColliderEntity.h"
#include "ModelHelper.h"
#include "DrawMesh.h"

//class constants go here

class TriPlay : public Game
{
public:
    TriPlay(GLprogram prog);
    void update(double dt);
    void draw();
private:
    shared<Entity> me;
    
    struct RenderData {
        GLprogram prog;
        GLuniform<mat4> mat;
        GLuniform<vec3> pos;
    };

    RenderData objectData, forwardData;

    struct LightData {
        Light::Point light;
        uint32_t index;
        Light::Group<Light::Point>* group;
    };

    LightData dLight, dLight2;
    void updateLights();

    void setupLights();
    void setupPostProcess();
    GLresource<GLtime> crtTime;
    GLresource<GLresolution> crtRes;
};
