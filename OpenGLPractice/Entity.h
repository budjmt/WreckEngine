#pragma once

#include "smart_ptr.h"

#include "Drawable.h"
#include "Transform.h"

enum EntityType {
	NORMAL,
	COLLIDER,
	CAMERA
};

class Entity
{
public:
	Entity() { };
	Entity(shared<Drawable> s);
	Entity(vec3 p, vec3 sc, vec3 rA, float r, shared<Drawable> s);

	Transform& transform = _transform;
	bool& active = _active;
	vec4& color = _color;

	void* id = (void*)rand();//meant to identify the object for debugging purposes

	virtual void update(double dt) { };
	virtual void draw();
protected:
	shared<Drawable> shape;
	vec4 _color = vec4(1);
	Transform _transform;
	bool _active = true;
	ACCS_G (EntityType, type);
};

