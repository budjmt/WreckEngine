#include "RigidBody.h"

short RigidBody::floatingObj() const { return _floatingObj; }; 
void RigidBody::floatingObj(short f) { _floatingObj = f; };
short RigidBody::staticObj() const { return _staticObj; }; 
void RigidBody::staticObj(short s) { _staticObj = s; _floatingObj = s; };

float RigidBody::mass() const { return _mass; }; 
void RigidBody::mass(float m) { _mass = m; _invMass = 1 / m; };
float RigidBody::invMass() const { return _invMass; };
float RigidBody::restitution() const { return _restitution; }
void RigidBody::restitution(float e) { _restitution = e; }

glm::vec3 RigidBody::vel() const { return _vel; }; 
void RigidBody::vel(glm::vec3 v) { _vel = v; _speed = glm::length(v); };
glm::vec3 RigidBody::heading() const { return _heading; };

glm::vec3 RigidBody::angVel() const { return _angVel; }; 
void RigidBody::angVel(glm::vec3 a) { _angVel = a; _angSpeed = glm::length(a); };
glm::vec3 RigidBody::angHeading() const { return _angHeading; };

void RigidBody::update(double dt) {
	updateVel(dt);
	updateAngVel(dt);

	netForce = glm::vec3(0, 0, 0);
	netAngAccel = glm::vec3(0, 0, 0);
}

void RigidBody::updateVel(double dt) {
	_vel += (1 - _staticObj) * _invMass * (float)dt * netForce;
	_speed = glm::length(_vel);

	if (_speed < MIN_VEL) {
		_vel = glm::vec3(0, 0, 0);
		_speed = 0;
	}
	else {
		if (_speed > MAX_VEL) {
			_vel *= MAX_VEL / _speed;
			_speed = MAX_VEL;
		}
		_heading = _vel / _speed;
	}
}

void RigidBody::updateAngVel(double dt) {
	_angVel += (1 - _staticObj) * (float)dt * netAngAccel;
	_angSpeed = glm::length(_angVel);

	if (_angSpeed < MIN_VEL) {
		_angVel = glm::vec3(0, 0, 0);
		_angSpeed = 0;
	}
	else {
		if (_angSpeed > MAX_VEL) {
			_angVel *= MAX_VEL / _angSpeed;
			_angSpeed = MAX_VEL;
		}
		_angHeading = _angVel / _angSpeed;
	}
}

glm::vec3 RigidBody::quadDrag(float c_d, glm::vec3 v, glm::vec3 h)
{
	return c_d * glm::dot(v, v) * h;
}
