#pragma once
#include "Game.h"

#include "glm/gtc/random.hpp"
#include "glm/gtc/constants.hpp"

#include "External.h"
#include "Camera.h"
#include "ColliderEntity.h"
#include "ModelHelper.h"
#include "DrawMesh.h"

//class constants go here

class TriPlay : public Game
{
public:
	TriPlay() = default;
	TriPlay(GLprogram prog);
	void update(double dt);
private:
	shared<Entity> me;
	shared<Camera> camera;
};

