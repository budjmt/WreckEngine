#include "ColliderEntity.h"

std::vector<ColliderEntity*> ColliderEntity::colliderEntities;

ColliderEntity::ColliderEntity(Drawable* s)
	: Entity(s)
{
	//_collider = new Collider(&transform,transform.scale);
	_collider = new Collider(((DrawMesh*)s)->mesh(), &transform);
	mass = 1;
	invMass = 1;
	_staticObj = 0;
	colliderEntities.push_back(this);
}

ColliderEntity::ColliderEntity(glm::vec3 p,glm::vec3 dims,glm::vec3 sc,glm::vec3 rA,float r,Drawable* s) 
	: Entity(p,sc,rA,r,s)
{
	//_collider = new Collider(&transform,dims);
	_collider = new Collider(((DrawMesh*)s)->mesh(),&transform);
	mass = 1;
	invMass = 1;
	_staticObj = 0;
	colliderEntities.push_back(this);
}

ColliderEntity::ColliderEntity(const ColliderEntity& other) 
	: Entity(other)
{
	_collider = other.collider();
	staticObj(other.staticObj());
	mass = other.mass;
	invMass = other.invMass;
	vel(other.vel());
	angVel(other.angVel());
	colliderEntities.push_back(this);
}


ColliderEntity::~ColliderEntity(void)
{
	delete _collider;
}

int ColliderEntity::staticObj() const { return _staticObj; } void ColliderEntity::staticObj(int s) { _staticObj = s; }
glm::vec3 ColliderEntity::vel() const { return _vel; } void ColliderEntity::vel(glm::vec3& v) { _vel = v; }
glm::vec3 ColliderEntity::angVel() const { return _angVel; } void ColliderEntity::angVel(glm::vec3& a) { _angVel = a; }
Collider* ColliderEntity::collider() const { return _collider; }

void ColliderEntity::update(double dt) {
	calcForces();
	_vel += netForce  * (float)dt;
	transform.position += _vel  * (float)dt;
	transform.rotate(_angVel * (float)dt);

	if (_vel.length() > MAX_VEL)
		_vel *= MAX_VEL / _vel.length();
	else if (_vel.length() < 0.05f) {
		_vel = glm::vec3(0, 0, 0);
		_angVel = glm::vec3(0, 0, 0);
	}
	netForce = glm::vec3(0, 0, 0);
}

#include <iostream>
#include "DebugBenchmark.h"

void ColliderEntity::calcForces() {
	//netForce += glm::vec3(0, mass * -9.8f * (1 - estaticObj), 0);//gravity
	//collision resolution stuff here
	
	//I need to fix this so that all the colliders are updated and THEN run collision checks
	//updateCorners();
	_collider->update();
	//DebugBenchmark::getInstance().start();
	for (ColliderEntity* entity : colliderEntities) {
		Collider* other = entity->collider();
		if (other == _collider || !entity->active)
			continue;
		Manifold m = _collider->intersects(other);
		if (m.originator != nullptr)
			//handleCollision(entity,);
			std::cout << "collision! " << _collider << ", " << m.originator << "; " << m.pen << std::endl;
	}
	//std::cout << "Collision Check Time: " << DebugBenchmark::getInstance().end() << std::endl;

	netForce += -0.15f * _vel * _vel * mass;//quadratic drag
	netForce *= invMass;
}

glm::vec3 ColliderEntity::calcTorque(glm::vec3 colPoint, glm::vec3 F) {
	glm::vec3 torque = glm::cross(colPoint, _angVel);
	return torque;
}

void ColliderEntity::handleCollision(ColliderEntity* other, glm::vec3 norm, float depth) {
	float velAlongNorm = glm::dot(other->vel() - _vel, norm);
	if (velAlongNorm > 0)
		return;

	float e = 1;
	float j = -(1 + e) * velAlongNorm;
	j /= invMass + other->invMass;

	glm::vec3 impulse = j * norm;
	float massRatio = mass / (mass + other->mass);
	//evel -= massRatio * impulse;
	massRatio *= other->mass * invMass;
	//other->vel += massRatio * impulse;

	//correct positions
	glm::vec3 correction = norm * 0.2f / (invMass + other->invMass);
	correction *= glm::max(depth - 0.05f, 0.0f);
	transform.position -= correction * invMass;
	other->transform.position += correction * other->invMass;
}