#include "ColliderEntity.h"


ColliderEntity::ColliderEntity(Drawable* s)
	: Entity(s)
{
	ecollider = new Collider(&transform,transform.scale);
	mass = 1;
	invMass = 1;
	estaticObj = 0;
}

ColliderEntity::ColliderEntity(glm::vec3 p,glm::vec3 dims,glm::vec3 sc,glm::vec3 rA,float r,Drawable* s) 
	: Entity(p,sc,rA,r,s)
{
	ecollider = new Collider(&transform,dims);
	mass = 1;
	invMass = 1;
	estaticObj = 0;
}

ColliderEntity::ColliderEntity(const ColliderEntity& other) 
	: Entity(other)
{
	ecollider = other.collider();
	staticObj(other.staticObj());
	mass = other.mass;
	invMass = other.invMass;
	vel(other.vel());
	angVel(other.angVel());
}


ColliderEntity::~ColliderEntity(void)
{
	delete ecollider;
}

int ColliderEntity::staticObj() const { return estaticObj; } void ColliderEntity::staticObj(int s) { estaticObj = s; }
glm::vec3 ColliderEntity::vel() const { return evel; } void ColliderEntity::vel(glm::vec3& v) { evel = v; }
glm::vec3 ColliderEntity::angVel() const { return eangVel; } void ColliderEntity::angVel(glm::vec3& a) { eangVel = a; }
Collider* ColliderEntity::collider() const { return ecollider; }

void ColliderEntity::update(double dt) {
	calcForces();
	evel += netForce  * (float)dt;
	transform.position += evel  * (float)dt;
	transform.rotate(eangVel * (float)dt);

	if (evel.length() > MAX_VEL)
		evel *= MAX_VEL / evel.length();
	else if (evel.length() < 0.05f) {
		evel = glm::vec3(0, 0, 0);
		eangVel = glm::vec3(0, 0, 0);
	}
	netForce = glm::vec3(0, 0, 0);
}

void ColliderEntity::calcForces() {
	netForce += glm::vec3(0, mass * -9.8f * (1 - estaticObj), 0);//gravity
	//collision resolution stuff here
	/*
	updateCorners();
	for(Entity* entity : entities) {
		if(entity != entity && entity.type == COLLIDER) {
			if(collider->intersects(entity->collider))
				handleCollision(entity,);
		}
	*/

	netForce += -0.15f * evel * evel * mass;//quadratic drag
	netForce *= invMass;
}

glm::vec3 ColliderEntity::calcTorque(glm::vec3 colPoint, glm::vec3 F) {
	glm::vec3 torque = glm::cross(colPoint, eangVel);
	return torque;
}

void ColliderEntity::handleCollision(ColliderEntity* other, glm::vec3 norm, float depth) {
	float velAlongNorm = glm::dot(other->vel() - evel, norm);
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