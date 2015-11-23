#include "Transform.h"

#include "glm/gtx/transform.hpp"

Transform::Transform()
	: position(tposition), scale(tscale), rotation(trotation), rotAngle(trotAngle), rotAxis(trotAxis)
{
	parent = nullptr;
	position = glm::vec3(0, 0, 0);
	scale = glm::vec3(1, 1, 1);
	rotation = glm::fquat();
	updateRot();
}

Transform::Transform(const Transform& other) 
	: position(tposition), scale(tscale), rotation(trotation), rotAngle(trotAngle), rotAxis(trotAxis)
{
	parent = other.parent;
	position = other.position;
	scale = other.scale;
	rotation = other.rotation;
	rotAxis = other.rotAxis;
	rotAngle = other.rotAngle;
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
	return *this;
}

Transform Transform::computeTransform() {
	if (parent == nullptr) {
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
	glm::mat4 m = glm::rotate(rotAngle, rotAxis);
	tforward = (glm::vec3)(m * glm::vec4(0, 0, 1, 1));
	tup = (glm::vec3)(m * glm::vec4(0, 1, 0, 1));
	tright = glm::cross(tforward, tup);
}

glm::vec3 Transform::forward() { return tforward; }
glm::vec3 Transform::up() { return tup; }
glm::vec3 Transform::right() { return tright; }

void Transform::updateRot() {
	rotAngle = glm::angle(rotation);
	rotAxis = glm::axis(rotation);
	updateNormals();
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

glm::vec3 Transform::getTransformed(glm::vec3 v)
{
	Transform t = computeTransform();
	glm::mat4 translate = glm::translate(tposition);
	glm::mat4 rot = glm::rotate(t.rotAngle, t.rotAxis);
	glm::mat4 scale = glm::scale(tscale);
	return (glm::vec3)(translate * scale * rot * glm::vec4(v, 1));
}
