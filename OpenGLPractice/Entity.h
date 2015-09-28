#pragma once

#include "Shape.h"

enum EntityType {
	NORMAL,
	COLLIDER
};

class Entity
{
public:
	//Entity();
	Entity(Drawable* s);
	Entity(glm::vec3 p,glm::vec3 sc,glm::vec3 rA,float r,Drawable* s);
	Entity(const Entity& other);
	~Entity(void);
	glm::vec3& pos;
	glm::vec3& scale;
	glm::vec3& rotAxis;
	float& rot;
	bool& active;
	glm::vec4& color;
	EntityType getType();
	virtual void update(double dt);
	void draw();
protected:
	Drawable* shape;
	glm::vec4 ecolor;
	glm::vec3 epos;
	glm::vec3 escale;
	glm::vec3 erotAxis;
	float erot;
	bool eactive = true;
	EntityType type;
};

