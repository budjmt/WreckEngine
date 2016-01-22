#include "Entity.h"

Entity::Entity() 
	: transform(_transform), active(_active)
{

}

Entity::Entity(Drawable* s)
	: transform(_transform), active(_active)
{
	shape = s;
	_color = glm::vec4(1, 1, 1, 1);
}

Entity::Entity(glm::vec3 p,glm::vec3 sc,glm::vec3 rA,float r,Drawable* s)
	: transform(_transform), active(_active)
{
	transform.position = p;
	transform.scale = sc;
	transform.rotate(r, rA);
	shape = s;
	_color = glm::vec4(1, 1, 1, 1);
}

Entity::Entity(const Entity& other) 
	: transform(_transform), active(_active)
{
	transform = other.transform;
	shape = other.shape;
	active = other.active;
	_color = other.color();
}


Entity::~Entity(void)
{
}

glm::vec4 Entity::color() const { return _color; } 
void Entity::color(glm::vec4& c) { _color = c; }

EntityType Entity::type() { return _type; }

void Entity::update(double dt) {
	//rot += (float)(dt * 10);
}

void Entity::draw() {
	glUniform4fv(shape->colorLoc, 1, &_color[0]);
	shape->draw(transform);
}