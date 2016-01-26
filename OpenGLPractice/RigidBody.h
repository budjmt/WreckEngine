#pragma once

#include "glm/glm.hpp"

const float MIN_VEL = 1.f;//it's strangely high
const float MAX_VEL = 40.f;

class RigidBody {
public:
	glm::vec3 netForce = glm::vec3(0, 0, 0), netAngAccel = glm::vec3(0, 0, 0);

	short floatingObj() const; void floatingObj(short f);
	short staticObj() const; void staticObj(short s);

	float mass() const; void mass(float m);
	float invMass() const;
	float restitution() const; void restitution(float e);

	glm::vec3 vel() const; void vel(glm::vec3 v);
	glm::vec3 heading() const;

	glm::vec3 angVel() const; void angVel(glm::vec3 a);
	glm::vec3 angHeading() const;

	void update(double dt);
	void updateVel(double dt);
	void updateAngVel(double dt);

	glm::vec3 quadDrag(float c_d, glm::vec3 v, glm::vec3 h);

private:
	short _floatingObj = 0
		, _staticObj = 0;
	float _mass = 1, _invMass = 1;
	float _speed = 0, _angSpeed = 0;
	float _restitution = 1;
	glm::vec3 _vel = glm::vec3(0, 0, 0), _heading = glm::vec3(1, 0, 0)
		, _angVel = glm::vec3(0, 0, 0), _angHeading = glm::vec3(1, 0, 0);
};