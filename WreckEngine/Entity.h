#pragma once

#include "smart_ptr.h"

#include "Random.h"

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
	Entity() = default;
	Entity(shared<Drawable> s);
	Entity(vec3 p, vec3 sc, vec3 rA, float r, shared<Drawable> s);

	Transform transform;
	bool active = true;
	vec4 color = vec4(1);

	void* id = (void*)Random::get();//meant to identify the object for debugging purposes

	virtual void update(double dt) { };
	virtual void draw();
protected:
	shared<Drawable> shape;
	ACCS_G (protected, EntityType, type);
};

class LogicEntity : public Entity {
	using update_func = void(*)(LogicEntity*, double);
public:
	LogicEntity(update_func f) : Entity(), custom_update(f) { }
	update_func custom_update;
	void update(double dt) { custom_update(this, dt); }
	void draw() {}
};