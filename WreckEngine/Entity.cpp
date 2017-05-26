#include "Entity.h"

Entity::Entity(shared<GraphicsWorker> s) : shape(s) { }
Entity::Entity(vec3 p, vec3 sc, vec3 rA, float r, shared<GraphicsWorker> s) : Entity(s)
{
    transform.position = p;
    transform.scale = sc;
    transform.rotate(r, rA);
}

void Entity::draw() {
    shape->draw(&transform, this);
}