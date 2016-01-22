#pragma once

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
	Entity();
	Entity(Drawable* s);
	Entity(glm::vec3 p,glm::vec3 sc,glm::vec3 rA,float r,Drawable* s);
	Entity(const Entity& other);
	~Entity(void);

	Transform& transform;
	bool& active;
	glm::vec4 color() const; void color(glm::vec4& c);
	EntityType type();

	virtual void update(double dt);
	virtual void draw();
protected:
	Drawable* shape;
	glm::vec4 _color;
	Transform _transform;
	bool _active = true;
	EntityType _type;
};

