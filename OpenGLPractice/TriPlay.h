#pragma once
#include "Game.h"

#include "glm/gtc/random.hpp"
#include "glm/gtc/constants.hpp"

#include "Mouse.h"
#include "Camera.h"
#include "ColliderEntity.h"
#include "ModelHelper.h"
#include "DrawMesh.h"

//class constants go here

class TriPlay :
	public Game
{
public:
	TriPlay();
	TriPlay(GLuint prog, GLFWwindow* w);
	TriPlay(const TriPlay& other);
	~TriPlay();
	void update(GLFWwindow* window, Mouse* m, double dt);
	//void spawnTriangle(glm::vec3 pos, glm::vec3 vel);
private:
	//std::vector<ColliderEntity*> triangles;
	Entity* me;
	std::vector<Entity*> meshes;
	Camera* camera;
	GLFWwindow* window;
};

