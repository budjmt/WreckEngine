#include "Entity.h"

Entity::Entity() 
	: transform(etransform), active(eactive)
{

}

Entity::Entity(Drawable* s)
	: transform(etransform), active(eactive)
{
	shape = s;
	ecolor = glm::vec4(1, 1, 1, 1);
}

Entity::Entity(glm::vec3 p,glm::vec3 sc,glm::vec3 rA,float r,Drawable* s)
	: transform(etransform), active(eactive)
{
	transform.position = p;
	transform.scale = sc;
	transform.rotate(r, rA);
	shape = s;
	ecolor = glm::vec4(1, 1, 1, 1);
}

Entity::Entity(const Entity& other) 
	: transform(etransform), active(eactive)
{
	transform = other.transform;
	shape = other.shape;
	active = other.active;
	ecolor = other.color();
}


Entity::~Entity(void)
{
}

glm::vec4 Entity::color() const { return ecolor; } 
void Entity::color(glm::vec4& c) { ecolor = c; }

EntityType Entity::type() { return etype; }

void Entity::update(double dt) {
	//rot += (float)(dt * 10);
}

void Entity::draw() {
	glUniform4fv(shape->colorLoc, 1, &ecolor[0]);
	shape->draw(transform);
}