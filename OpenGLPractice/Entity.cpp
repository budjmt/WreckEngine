#include "Entity.h"

Entity::Entity(shared<Drawable> s) : shape(s) { }
Entity::Entity(vec3 p, vec3 sc, vec3 rA, float r, shared<Drawable> s) : Entity(s)
{
	transform.position = p;
	transform.scale = sc;
	transform.rotate(r, rA);
}

void Entity::draw() {
	shape->colorLoc.update(_color);
	shape->draw(&transform);
}