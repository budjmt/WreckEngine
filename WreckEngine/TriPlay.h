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
    GLprogram objectProgram;
    GLuniform<mat4> objectCamera;

    void setupLights();
    void setupPostProcess();
    GLresource<GLtime> crtTime;
    GLresource<GLresolution> crtRes;
};
