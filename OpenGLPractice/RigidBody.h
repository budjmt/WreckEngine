#pragma once

#include "glm/glm.hpp"

const float MIN_VEL = 1.f;//it's strangely high
const float MAX_VEL = 40.f;

class RigidBody {
public:
	glm::vec3 netForce = glm::vec3(0, 0, 0), netAngAccel = glm::vec3(0, 0, 0);

	short floatingObj() const { return _floatingObj; }; void floatingObj(short f) { _floatingObj = f; };
	short staticObj() const { return _staticObj; }; void staticObj(short s) { _staticObj = s; _floatingObj = s; };

	float mass() const { return _mass; }; void mass(float m) { _mass = m; _invMass = 1 / m; };
	float invMass() const { return _invMass; };

	glm::vec3 vel() const { return _vel; }; void vel(glm::vec3 v) { _vel = v; _speed = glm::length(v); };
	glm::vec3 heading() const { return _heading; };

	glm::vec3 angVel() const { return _angVel; }; void angVel(glm::vec3 a) { _angVel = a; _angSpeed = glm::length(a); };
	glm::vec3 angHeading() const { return _angHeading; };

	void update(double dt);
	void updateVel(double dt);
	void updateAngVel(double dt);

	glm::vec3 quadDrag(float c_d, glm::vec3 v, glm::vec3 h);

private:
	short _floatingObj = 0
		, _staticObj = 0;
	float _mass = 1, _invMass = 1;
	float _speed = 0, _angSpeed = 0;
	glm::vec3 _vel = glm::vec3(0, 0, 0), _heading = glm::vec3(1, 0, 0)
		, _angVel = glm::vec3(0, 0, 0), _angHeading = glm::vec3(1, 0, 0);
};