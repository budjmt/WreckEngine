#include "RigidBody.h"
#include <cassert>

void RigidBody::update(double dt) {
	updateVel(dt);
	updateAngVel(dt);

	netForce = vec3();
	netAngAccel = vec3();
}

void RigidBody::updateVel(double dt) {
	vel += _invMass * (float)dt * netForce;

	if (_speed < MIN_VEL) {
		_vel = vec3();
		_speed = 0;
	}
	else {
		if (_speed > MAX_VEL) {
			_vel *= MAX_VEL / _speed;
			_speed = MAX_VEL;
		}
		_heading = _vel / _speed;
	}
	//this is enough to determine if there's an issue
	assert(!NaN_CHECK(_speed));
}

void RigidBody::updateAngVel(double dt) {
	angVel += (float)dt * netAngAccel;

	if (_angSpeed < MIN_VEL) {
		_angVel = vec3();
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

void RigidBody::applyGravity() {
	const float g = 9.8f;
	netForce += vec3(0, _mass * -g * (1 - _floating), 0);
}

//the coefficient here is equivalent to 0.5 * density of fluid (here just air) * C_d (drag coefficient), which we boil down to C_d
//C_d is dependent on the object's shape and the Reynolds Number, R_e = internal forces / viscous forces = mag(v) * D / viscosity, 
//where D is some characteristic diameter or linear dimension and viscosity is the kinematic viscosity = viscosity / density
vec3 RigidBody::quadDrag(float c_d, vec3 v, vec3 h)
{
	return c_d * glm::dot(v, v) * h;
}