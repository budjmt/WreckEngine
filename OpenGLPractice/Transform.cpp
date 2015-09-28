#include "Transform.h"

#include "glm/gtx/transform.hpp"

Transform::Transform()
	: position(tposition), scale(tscale), rotAxis(trotAxis), rotation(trotation), forward(tforward), up(tup), right(tright)
{
	position = glm::vec3(0, 0, 0);
	scale = glm::vec3(1, 1, 1);
	rotAxis = glm::vec3(0, 0, 1);
	rotation = 0;
}

Transform::Transform(const Transform& other) 
	: position(tposition), scale(tscale), rotAxis(trotAxis), rotation(trotation), forward(tforward), up(tup), right(tright)
{
	position = other.position;
	scale = other.scale;
	rotAxis = other.rotAxis;
	rotation = other.rotation;
	forward = other.forward;
	up = other.up;
	right = other.right;
}


Transform::~Transform()
{
}

Transform& Transform::operator=(const Transform& other) 
{
	position = other.position;
	scale = other.scale;
	rotAxis = other.rotAxis;
	rotation = other.rotation;
	forward = other.forward;
	up = other.up;
	right = other.right;
}

Transform Transform::computeTransform() {
	if (!parent)
		return *this;
	Transform t = Transform();
	t.position = parent->position + position;
	t.scale = parent->scale * scale;
	//I'm not including this until quaternions
	//t.rotation = parent->rotation + rotation;
	t.rotAxis = rotAxis;
	t.rotation = rotation;
	t.parent = parent->parent;
	return t.computeTransform();
}

void Transform::updateNormals() {

}

glm::vec3 Transform::getForward() {
	glm::mat4 m = glm::translate(position) * glm::rotate(rotation, rotAxis);
	return (glm::vec3)(m * glm::vec4(0, 0, 1, 1));
}

glm::vec3 Transform::getUp() {
	glm::mat4 m = glm::translate(position) * glm::rotate(rotation, rotAxis);
	return (glm::vec3)(m * glm::vec4(0, 1, 0, 1));
}

glm::vec3 Transform::getRight() {
	return glm::cross(getForward(), getUp());
}