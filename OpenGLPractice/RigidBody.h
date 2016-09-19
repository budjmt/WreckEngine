#pragma once

#include "MarchMath.h"

const float MIN_VEL = .1f;//it's strangely high
const float MAX_VEL = 40.f;

class RigidBody {
public:
	RigidBody() { }

	vec3 netForce, netAngAccel;

	float floating() const; void floating(float f);
	float fixed() const; void fixed(float s);
	short solid() const; void solid(short s);

	float mass() const; void mass(float m);
	float invMass() const;
	float restitution() const; void restitution(float e);

	vec3 vel() const; void vel(vec3 v);
	vec3 heading() const;

	vec3 angVel() const; void angVel(vec3 a);
	vec3 angHeading() const;

	void update(double dt);
	void updateVel(double dt);
	void updateAngVel(double dt);

	void applyGravity();
	vec3 quadDrag(float c_d, vec3 v, vec3 h);

private:
	float _floating = 0, _fixed = 0; 
	short _solid = 1;

	float _mass = 1, _invMass = 1;
	float _restitution = 1;
	
	vec3 _vel,    _heading    = vec3(1, 0, 0)
	   , _angVel, _angHeading = vec3(1, 0, 0);
	float _speed = 0, _angSpeed = 0;
};