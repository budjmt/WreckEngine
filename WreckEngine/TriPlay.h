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
    TriPlay();
    void update(double dt);
    void postUpdate();
    void physicsUpdate(double dt);
    void draw();
private:
    shared<Entity> me;

    struct LightData {
        Light::Point light;
        Light::light_key<Light::Point> key;
        Light::group_proxy<Light::Point> group;
    };

    LightData dLight, dLight2;
    void updateLights();

    void setupLights();
    void setupPostProcess();
};
