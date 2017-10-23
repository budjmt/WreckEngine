#pragma once
#include "Game.h"

#include "External.h"
#include "Text.h"
#include "Camera.h"
#include "ColliderEntity.h"
#include "ModelHelper.h"
#include "DrawMesh.h"

//class constants go here

class SimpleGame : public Game
{
public:
    SimpleGame(GLprogram prog);
    void update(double dt) override;
private:
    shared<Entity> me;
};
