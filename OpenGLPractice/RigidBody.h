#pragma once

#include "MarchMath.h"
#include "property.h"

const float MIN_VEL = .1f;//it's strangely high
const float MAX_VEL = 40.f;

class RigidBody {
public:
	vec3 netForce, netAngAccel;

	void update(double dt);
	void updateVel(double dt);
	void updateAngVel(double dt);

	void applyGravity();
	vec3 quadDrag(float c_d, vec3 v, vec3 h);

private:
	PROP_GS_D_S   (float, floating, 0);
	PROP_GS_D_S   (short, solid, 1);
	PROP_GS_D_S_C (float, fixed, { return _fixed; }, { _fixed = value; _floating = value; }, 0);

	PROP_GS_D_S_C (float, mass, { return _mass; }, { _mass = value; _invMass = 1.f / value; }, 1);
	PROP_G_D_S    (float, invMass, 1);
	PROP_GS_D_S   (float, restitution, 1);
	
	float _speed = 0, _angSpeed = 0;
	PROP_GS_S_C(vec3, vel,    { return _vel;    }, { _vel = value;    _speed = glm::length(value); });
	PROP_GS_S_C(vec3, angVel, { return _angVel; }, { _angVel = value; _angSpeed = glm::length(value); });
	PROP_G_D_S(vec3, heading, vec3(1, 0, 0));
	PROP_G_D_S(vec3, angHeading, vec3(1, 0, 0));
};