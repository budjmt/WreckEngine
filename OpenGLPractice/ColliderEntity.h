#pragma once
#include "entity.h"
#include "Collider.h"

const float MAX_VEL = 20.f;

class ColliderEntity :
	public Entity
{
public:
	ColliderEntity(Drawable* s);
	ColliderEntity(glm::vec3 p,glm::vec3 dims,glm::vec3 sc,glm::vec3 rA,float r,Drawable* s);
	ColliderEntity(const ColliderEntity& other);
	~ColliderEntity(void);
	int staticObj() const; void staticObj(int s);
	glm::vec3 vel() const; void vel(glm::vec3& v);
	glm::vec3 angVel() const; void angVel(glm::vec3& a);
	Collider* collider() const;
	virtual void update(double dt);
	virtual void calcForces();
	glm::vec3 calcTorque(glm::vec3 colPoint, glm::vec3 F);
	void handleCollision(ColliderEntity* other, glm::vec3 norm, float depth);
private:
	Collider* ecollider;
	int estaticObj;
	float mass, invMass;
	float inertia, invIntertia;
	glm::vec3 evel, eangVel, netForce, netTorque;
};

