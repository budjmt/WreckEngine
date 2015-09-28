#pragma once
#include "Entity.h"
class Camera :
	public Entity
{
public:
	Camera();
	~Camera();
	void update(double dt);
	void draw();
	void turn(float dx,float dy);
	glm::vec3 getLocation();
	glm::vec3 getForward();
	glm::vec3 getLookAt();
	glm::vec3 getUp();
	glm::vec3 getRight();
private:
	float yaw, pitch;
};

