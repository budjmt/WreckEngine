#pragma once
#include <memory>

#include "entity.h"
#include "RigidBody.h"
#include "Collider.h"
#include "DrawMesh.h"

class ColliderEntity :
	public Entity
{
public:
	ColliderEntity(Drawable* s);
	ColliderEntity(glm::vec3 p,glm::vec3 dims,glm::vec3 sc,glm::vec3 rA,float r,Drawable* s);
	~ColliderEntity();
	ColliderEntity(const ColliderEntity& other);
	ColliderEntity& operator=(ColliderEntity& other);
	
	RigidBody& rigidBody();
	Collider* collider() const; void collider(Collider* c);
	virtual void update(double dt);
	virtual void calcForces(double dt);
	void handleCollision(ColliderEntity* other, Manifold& m, double dt);
	glm::vec3 calcAngularAccel(Manifold& m, glm::vec3 F);
private:
	Collider* _collider;
	RigidBody body;
	
	static std::vector<ColliderEntity*> colliderEntities;
};