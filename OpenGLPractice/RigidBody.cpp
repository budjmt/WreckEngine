#include "RigidBody.h"

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
