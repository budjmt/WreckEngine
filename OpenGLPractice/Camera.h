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
	glm::vec3 getLocation();//necessary?
	glm::vec3 getForward();//n?
	glm::vec3 getLookAt();
	glm::vec3 getUp();//n?
	glm::vec3 getRight();//n?
private:
	float yaw, pitch;
};

