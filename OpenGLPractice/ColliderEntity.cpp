#include "ColliderEntity.h"

std::vector<ColliderEntity*> ColliderEntity::colliderEntities;

ColliderEntity::ColliderEntity(Drawable* s)
	: Entity(s)
{
	//_collider = new Collider(&transform,transform.scale);
	_collider = new Collider(((DrawMesh*)s)->mesh(), &transform);
	colliderEntities.push_back(this);
}

ColliderEntity::ColliderEntity(glm::vec3 p,glm::vec3 dims,glm::vec3 sc,glm::vec3 rA,float r,Drawable* s) 
	: Entity(p,sc,rA,r,s)
{
	//_collider = new Collider(&transform,dims);
	_collider = new Collider(((DrawMesh*)s)->mesh(),&transform);
	colliderEntities.push_back(this);
}

ColliderEntity::~ColliderEntity() {
	delete _collider;
}

ColliderEntity::ColliderEntity(const ColliderEntity& other) {
	collider(new Collider(*other.collider()));
	body = other.body;
}

ColliderEntity& ColliderEntity::operator=(ColliderEntity& other) {
	collider(other.collider());
	other._collider = nullptr;
	body = other.body;
	return *this;
}

RigidBody& ColliderEntity::rigidBody() { return body; }
Collider* ColliderEntity::collider() const { return _collider; } void ColliderEntity::collider(Collider* c) { _collider = c; }

void ColliderEntity::update(double dt) {
	calcForces(dt);
	body.update(dt);
	transform.position += body.vel()  * (float)dt;
	transform.rotate(body.angVel() * (float)dt);
}

#include <iostream>
#include "DebugBenchmark.h"

void ColliderEntity::calcForces(double dt) {
	body.netForce += glm::vec3(0, body.mass() * -9.8f * 0.5f * (1 - body.floatingObj()), 0);//gravity
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
		if (m.originator != nullptr) {
			if(m.originator == _collider)
				handleCollision(entity,m,dt);
			else
				entity->handleCollision(this,m,dt);
			_collider->update();
			entity->collider()->update();
			std::cout << "collision! " << _collider << ", " << m.originator << "; " << m.pen << std::endl;
		}
	}
	//std::cout << "Collision Check Time: " << DebugBenchmark::getInstance().end() << std::endl;

	//the coefficient here is equivalent to 0.5 * density of fluid (here just air) * C_d (drag coeff)
	//C_d is dependent on the object's shape and the Reynolds Number, R_e = internal forces / viscous forces = mag(v) * D / visc, 
	//where D is some characteristic diameter or linear dimension and visc is the kinematic viscosity = viscosity / density
	body.netForce += body.quadDrag(-0.15f, body.vel(), body.heading());//quadratic drag, no mass involved, it's all velocity dependent
	body.netAngAccel += body.quadDrag(-0.15f, body.angVel(), body.angHeading());//for ang accel too
	
	//body.netForce *= body.invMass();
}

void ColliderEntity::handleCollision(ColliderEntity* other, Manifold& m, double dt) {
	RigidBody oRB = other->rigidBody();
	float velAlongAxis = glm::dot(oRB.vel() - body.vel(), m.axis);
	//if the two bodies are travelling in the same direction along the axis
	if (velAlongAxis > 0)
		return;

	//coefficient of restitution. we take the min of the two coeffs
	//when e = 0, it is a perfect inelastic/plastic collision, and the objects stick together
	//when 0 < e < 1, it is a regular inelastic collision, with some energy dissipated
	//when e = 1, it is an elastic collision, where all energy is put into the response
	float e = glm::min(body.restitution(), oRB.restitution());

	//j = magnitude of impulse
	float j = velAlongAxis;
	j *= -(1 + e);
	j /= body.invMass() + oRB.invMass();

	//glm::vec3 impulse = j * m.axis;
	//float massRatio = mass / (mass + other->mass);
	//_vel -= massRatio * impulse;
	//massRatio *= other->mass * invMass;
	//other->vel(massRatio * impulse);

	//F is the force applied by the collision; we use the definition F = dp / dt, where p = momentum and dp = impulse
	j /= (float)dt;
	glm::vec3 F = j * m.axis;
	body.netForce += F;
	oRB.netForce += -F;
	
	//they have the same collision points by definition, but vecs to those points change, meaning torque and covariance also change
	body.netAngAccel += calcAngularAccel(m, F);
	oRB.netAngAccel += other->calcAngularAccel(m, -F);

	//correct positions
	float percent = 1.2f, slop = 0.05f;
	glm::vec3 correction = glm::max(-m.pen - slop, 0.0f) * percent * (1 + body.staticObj() + oRB.staticObj()) / (body.invMass() + oRB.invMass()) * m.axis;
	transform.position -= (body.invMass() + oRB.staticObj() * oRB.invMass()) * (1 - body.staticObj()) * correction;
	other->transform.position += (oRB.invMass() + body.staticObj() * body.invMass()) * (1 - oRB.staticObj()) * correction;
}

//Given a collision force F, calculates the change in angular acceleration it causes
glm::vec3 ColliderEntity::calcAngularAccel(Manifold& m, glm::vec3 F) {
	glm::vec3 torque = glm::vec3();
	if (!m.colPoints.size() || body.staticObj())
		return torque;

	glm::mat3 C = glm::mat3(0);//mass-weighted covariance

	//assumes uniform mass distribution; we can account for non-uniform distributions with constraints
	float m_n = body.mass() / m.colPoints.size();
	for (auto colPoint : m.colPoints) {
		glm::vec3 r = colPoint - _collider->framePos();//vector from the center of mass to the collision point
		torque += glm::cross(r, F);//torque = r x F = |r||F|sin(theta)
		C += m_n * glm::mat3(r.x * r, r.y * r, r.z * r);//m_n * r * r_transpose
	}
	float trace_C = C[0][0] + C[1][1] + C[2][2];

	glm::mat3 iT = glm::mat3() * trace_C - C;//inertia tensor = Identity_3x3 * trace(C) - C
	
	glm::vec3 at_iT = glm::vec3(m.axis.x * (iT[0][0] + iT[0][1] + iT[0][2])
		, m.axis.y * (iT[1][0] + iT[1][1] + iT[1][2])
		, m.axis.z * (iT[2][0] + iT[2][1] + iT[2][2]));//axis_transpose * inertia tensor (matrices are column major)
	
	float inertia = glm::dot(at_iT, m.axis);//axis_transpose * iT * axis = (axis_transpose * inertia tensor) . axis
	
	return (inertia) ? torque / inertia : glm::vec3();
}