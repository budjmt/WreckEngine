#include "Entity.h"

Entity::Entity() 
	: transform(etransform), active(eactive), color(ecolor)
{

}

Entity::Entity(Drawable* s)
	: transform(etransform), active(eactive), color(ecolor)
{
	shape = s;
	color = glm::vec4(1, 1, 1, 1);
}

Entity::Entity(glm::vec3 p,glm::vec3 sc,glm::vec3 rA,float r,Drawable* s)
	: transform(etransform), active(eactive), color(ecolor)
{
	transform.position = p;
	transform.scale = sc;
	transform.rotate(r, rA);
	shape = s;
	color = glm::vec4(1, 1, 1, 1);
}

Entity::Entity(const Entity& other) 
	: transform(etransform), active(eactive), color(ecolor)
{
	transform = other.transform;
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
	glUniform4fv(shape->colorLoc, 1, &color[0]);
	shape->draw(transform);
}