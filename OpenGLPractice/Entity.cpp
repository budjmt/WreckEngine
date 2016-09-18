#include "Entity.h"

Entity::Entity() { }
Entity::Entity(shared<Drawable> s) : shape(s) { }
Entity::Entity(vec3 p, vec3 sc, vec3 rA, float r, shared<Drawable> s) : Entity(s)
{
	transform.position(p);
	transform.scale(sc);
	transform.rotate(r, rA);
}

vec4 Entity::color() const { return _color; } 
void Entity::color(vec4& c) { _color = c; }

EntityType Entity::type() { return _type; }

void Entity::update(double dt) {
	//rot += (float)(dt * 10);
}

void Entity::draw() {
	shape->colorLoc.update(_color);
	shape->draw(transform.getComputed());
}