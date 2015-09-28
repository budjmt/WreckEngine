#pragma once
#include "Game.h"

#include "glm/gtc/random.hpp"
#include "glm/gtc/constants.hpp"

#include "Mouse.h"
#include "ColliderEntity.h"
#include "ModelHelper.h"

//class constants go here

class TriPlay :
	public Game
{
public:
	TriPlay();
	TriPlay(GLuint prog, int width, int height);
	TriPlay(const TriPlay& other);
	~TriPlay();
	void update(GLFWwindow* window, Mouse* m, double prevFrame, double dt);
	//void spawnTriangle(glm::vec3 pos, glm::vec3 vel);
private:
	//std::vector<ColliderEntity*> triangles;
	Entity* mesh;
};

