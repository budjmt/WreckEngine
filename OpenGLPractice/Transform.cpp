#include "Transform.h"

#include "glm/gtx/transform.hpp"

Transform::Transform()
	: position(tposition), scale(tscale), rotation(trotation), rotAngle(trotAngle), rotAxis(trotAxis), forward(tforward), up(tup), right(tright)
{
	position = glm::vec3(0, 0, 0);
	scale = glm::vec3(1, 1, 1);
	rotation = glm::fquat();
	updateRot();
}

Transform::Transform(const Transform& other) 
	: position(tposition), scale(tscale), rotation(trotation), rotAngle(trotAngle), rotAxis(trotAxis), forward(tforward), up(tup), right(tright)
{
	position = other.position;
	scale = other.scale;
	rotation = other.rotation;
	rotAxis = other.rotAxis;
	rotAngle = other.rotAngle;
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
	rotation = other.rotation;
	rotAxis = other.rotAxis;
	rotAngle = other.rotAngle;
	forward = other.forward;
	up = other.up;
	right = other.right;
	return *this;
}

Transform Transform::computeTransform() {
	if (!parent) {
		this->updateRot();
		return *this;
	}
	Transform t = Transform();
	t.position = parent->position + position;
	t.scale = parent->scale * scale;
	t.rotation = rotation * parent->rotation;
	t.parent = parent->parent;
	return t.computeTransform();
}

void Transform::updateNormals() {

}

glm::vec3 Transform::getForward() {
	glm::mat4 m = glm::rotate(rotAngle, rotAxis);
	return (glm::vec3)(m * glm::vec4(0, 0, 1, 1));
}

glm::vec3 Transform::getUp() {
	glm::mat4 m = glm::rotate(rotAngle, rotAxis);
	return (glm::vec3)(m * glm::vec4(0, 1, 0, 1));
}

glm::vec3 Transform::getRight() {
	return glm::cross(getForward(), getUp());
}

void Transform::updateRot() {
	rotAngle = glm::angle(rotation);
	rotAxis = glm::axis(rotation);
}

void Transform::rotate(float x, float y, float z) {
	if(x)
		rotation = glm::rotate(rotation, x, glm::vec3(1, 0, 0));
	if(y)
		rotation = glm::rotate(rotation, y, glm::vec3(0, 1, 0));
	if(z)
		rotation = glm::rotate(rotation, z, glm::vec3(0, 0, 1));
	updateRot();
}

void Transform::rotate(glm::vec3 v) {
	if (v.x)
		rotation = glm::rotate(rotation, v.x, glm::vec3(1, 0, 0));
	if (v.y)
		rotation = glm::rotate(rotation, v.y, glm::vec3(0, 1, 0));
	if (v.z)
		rotation = glm::rotate(rotation, v.z, glm::vec3(0, 0, 1));
	updateRot();
}

void Transform::rotate(float theta, glm::vec3 axis) {
	if (theta)
		rotation = glm::rotate(rotation, theta, axis);
	updateRot();
}