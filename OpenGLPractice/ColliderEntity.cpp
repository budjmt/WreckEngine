#include "ColliderEntity.h"


ColliderEntity::ColliderEntity(Shape* s)
	: Entity(s), staticObj(estaticObj)
	, vel(evel), angVel(eangVel), collider(ecollider)
{
	collider = new Collider(pos,scale);
	mass = 1;
	staticObj = 0;
}

ColliderEntity::ColliderEntity(glm::vec3 p,glm::vec3 dims,glm::vec3 sc,glm::vec3 rA,float r,Shape* s) 
	: Entity(p,sc,rA,r,s), staticObj(estaticObj)
	, vel(evel), angVel(eangVel), collider(ecollider)
{
	collider = new Collider(pos,dims);
	mass = 1;
	staticObj = 0;
}

ColliderEntity::ColliderEntity(const ColliderEntity& other) 
	: Entity(other), staticObj(estaticObj)
	, vel(evel), angVel(eangVel), collider(ecollider)
{
	collider = other.collider;
	staticObj = other.staticObj;
	mass = other.mass;
	staticObj = other.staticObj;
	vel = other.vel;
	angVel = other.angVel;
}


ColliderEntity::~ColliderEntity(void)
{
	delete collider;
}

void ColliderEntity::update(double dt) {
	calcForces();
	vel += netForce  * (float)dt;
	pos += vel  * (float)dt;
	rot += angVel.length() * (float)dt;
	//lazy fix is lazy also wat why is it using open gl coordinates
	if (pos.x > 1 || pos.x < -1) {
		vel.x *= -1;
		angVel.x *= -1;
	}
	if (pos.y > 1 || pos.y < -1) {
		vel.y *= -1;
		angVel.y *= -1;
	}
	if (vel.length() > MAX_VEL)
		vel *= MAX_VEL / vel.length();
	else if (vel.length() < 0.05f) {
		vel = glm::vec3(0, 0, 0);
		angVel = glm::vec3(0, 0, 0);
	}
	netForce = glm::vec3(0, 0, 0);
}

void ColliderEntity::calcForces() {
	netForce += glm::vec3(0, mass * -9.8f * (1 - staticObj), 0);//gravity
	//collision resolution stuff here
	/*
	updateCorners();
	for(Entity* entity : entities) {
		if(entity != entity && entity.type == COLLIDER) {
			if(collider->intersects(entity->collider))
				handleCollision(entity,);
		}
	*/

	netForce += -0.15f * vel * vel * mass;//quadratic drag
	netForce /= mass;
}

glm::vec3 ColliderEntity::calcTorque(glm::vec3 colPoint, glm::vec3 F) {
	//not even gonna touch this right now
	return glm::vec3(0, 0, 0);
}

void ColliderEntity::handleCollision(ColliderEntity* other, glm::vec3 norm, float depth) {
	float velAlongNorm = glm::dot(other->vel - vel, norm);
	if (velAlongNorm > 0)
		return;

	float e = 1;
	float j = -(1 + e) * velAlongNorm;
	j /= 1 / mass + 1 / other->mass;

	glm::vec3 impulse = j * norm;
	float massRatio = mass / (mass + other->mass);
	vel -= massRatio * impulse;
	massRatio *= other->mass / mass;
	other->vel += massRatio * impulse;

	//correct positions
	glm::vec3 correction = norm * 0.2f / (1 / mass + 1 / other->mass);
	correction *= glm::max(depth - 0.05f, 0.0f);
	pos -= correction / mass;
	other->pos += correction / other->mass;
}

void ColliderEntity::updateCorners() {
	std::vector<glm::vec3> corners;
	glm::vec3 toCorner = collider->dims * 0.5f;
	glm::vec3 toOppCorner = glm::vec3(toCorner.x, -toCorner.y, 0);
	toCorner = toCorner * glm::cos(rot) + glm::vec3(-toCorner.y,toCorner.x,0) * glm::sin(rot);
	toOppCorner = toOppCorner * glm::cos(rot) + glm::vec3(-toOppCorner.y,toOppCorner.x,0) * glm::sin(rot);
	//top left
	corners.push_back(pos - toCorner);
	//top right
	corners.push_back(pos + toOppCorner);
	//bottom right
	corners.push_back(pos + toCorner);
	//bottom left
	corners.push_back(pos - toOppCorner);
	collider->setCorners(corners);
}