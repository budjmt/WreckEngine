#pragma once

#include "MarchMath.h"
#include "property.h"

const float MIN_VEL = .001f;
const float MAX_VEL = 1000.f;

class RigidBody {
public:
	vec3 netForce, netAngAccel;

	void update(double dt);
	void updateVel(double dt);
	void updateAngVel(double dt);

	void applyGravity();
	vec3 quadDrag(float c_d, vec3 v, vec3 h);

private:
	ACCS_GS   (private, float, floating) = 0;
	ACCS_GS   (private, short, solid)    = 1;

	ACCS_GS_C (private, float, mass, { return _mass; }, { _mass = value; _invMass = value ? 1.f / value : 0; }) = 1; // infinite mass is represented by 0; this means gravity won't work
	ACCS_G    (private, float, invMass) = 1;
	// 0 is perfectly inelastic, i.e. objects stick together, 1 is perfectly elastic, i.e. objects bounce apart entirely
	ACCS_GS   (private, float, restitution) = 0.8f;
	
	float _speed = 0, _angSpeed = 0;
	PROP_GS (private, RigidBody, vec3, vel,    { return _vel;    }, { _speed = glm::length(value);    return _vel = value; });
	PROP_GS (private, RigidBody, vec3, angVel, { return _angVel; }, { _angSpeed = glm::length(value); return _angVel = value; });
    ACCS_G(private, vec3, heading)    { 1, 0, 0 };
    ACCS_G(private, vec3, angHeading) { 1, 0, 0 };
};