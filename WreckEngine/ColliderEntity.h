#pragma once
#include <memory>

#include "unique_id.h"
#include "smart_ptr.h"

#include "Entity.h"

#include "RigidBody.h"
#include "Collider.h"

#include "DrawMesh.h"

class ColliderEntity : public Entity
{
public:
	ColliderEntity(shared<DrawMesh> s);
	ColliderEntity(vec3 p, vec3 dims, vec3 sc, vec3 rA, float r, shared<DrawMesh> s);

	RigidBody& rigidBody = body;

	virtual void physicsUpdate(double dt);
	virtual void calcForces(double dt);
	virtual void handleCollision(ColliderEntity* other, Manifold& m, double dt, size_t& numCollisions);
	vec3 calcAngularAccel(Manifold& m, vec3 F);
protected:
	ACCS_G_T_C (protected, unique<Collider>, Collider*, collider, { return _collider.get(); });
	RigidBody body;
};

PARENT_TYPE(ColliderEntity, Entity);