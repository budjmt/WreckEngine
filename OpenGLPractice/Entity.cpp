#include "Entity.h"


Entity::Entity(Drawable* s)
	: pos(epos), scale(escale), rotAxis(erotAxis), rot(erot), active(eactive), color(ecolor)
{
	pos = glm::vec3(0,0,0);
	scale = glm::vec3(1,1,1);
	rotAxis = glm::vec3(0,0,1);
	rot = 0;
	shape = s;
	color = glm::vec4(1, 1, 1, 1);
}

Entity::Entity(glm::vec3 p,glm::vec3 sc,glm::vec3 rA,float r,Drawable* s)
	: pos(epos), scale(escale), rotAxis(erotAxis), rot(erot), active(eactive), color(ecolor)
{
	pos = p;
	scale = sc;
	rotAxis = rA;
	rot = r;
	shape = s;
	color = glm::vec4(1, 1, 1, 1);
}

Entity::Entity(const Entity& other) 
	: pos(epos), scale(escale), rotAxis(erotAxis), rot(erot), active(eactive), color(ecolor)
{
	pos = other.pos;
	scale = other.scale;
	rotAxis = other.rotAxis;
	rot = other.rot;
	shape = other.shape;
	active = other.active;
	color = other.color;
}


Entity::~Entity(void)
{
}

EntityType Entity::getType() {
	return type;
}

void Entity::update(double dt) {
	//rot += (float)(dt * 10);
}

void Entity::draw() {
	if (!shape)
		return;
	glUniform4fv(shape->colorLoc, 1, &color[0]);
	shape->draw(pos,scale,rotAxis,rot);
}