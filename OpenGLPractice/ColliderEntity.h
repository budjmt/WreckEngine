#pragma once
#include "entity.h"
#include "Collider.h"

const float MAX_VEL = 20.f;

class ColliderEntity :
	public Entity
{
public:
	ColliderEntity(Shape* s);
	ColliderEntity(glm::vec3 p,glm::vec3 dims,glm::vec3 sc,glm::vec3 rA,float r,Shape* s);
	ColliderEntity(const ColliderEntity& other);
	~ColliderEntity(void);
	int& staticObj;
	glm::vec3& vel;
	glm::vec3& angVel;
	Collider*& collider;
	virtual void update(double dt);
	virtual void calcForces();
	glm::vec3 calcTorque(glm::vec3 colPoint, glm::vec3 F);
	void handleCollision(ColliderEntity* other, glm::vec3 norm, float depth);

	void updateCorners();
private:
	Collider* ecollider;
	int estaticObj;
	float mass, inertia;
	glm::vec3 evel, eangVel, netForce, netTorque;
};

