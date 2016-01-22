#include "Transform.h"

#include "glm/gtx/transform.hpp"

Transform::Transform()
	: position(_position), scale(_scale), rotation(_rotation), rotAngle(_rotAngle), rotAxis(_rotAxis)
{
	parent = nullptr;
	_position = glm::vec3(0, 0, 0);
	_scale = glm::vec3(1, 1, 1);
	_rotation = glm::fquat();
	updateRot();
}

Transform::Transform(const Transform& other) 
	: position(_position), scale(_scale), rotation(_rotation), rotAngle(_rotAngle), rotAxis(_rotAxis)
{
	parent = other.parent;
	_position = other.position;
	_scale = other.scale;
	_rotation = other.rotation;
	_rotAxis = other.rotAxis;
	_rotAngle = other.rotAngle;
	_forward = other.forward();
	_up = other.up();
	_right = other.right();
}


Transform::~Transform()
{
}

Transform& Transform::operator=(const Transform& other) 
{
	_position = other.position;
	_scale = other.scale;
	_rotation = other.rotation;
	_rotAxis = other.rotAxis;
	_rotAngle = other.rotAngle;
	_forward = other.forward();
	_up = other.up();
	_right = other.right();
	return *this;
}

Transform Transform::computeTransform() {
	if (parent == nullptr) {
		this->updateRot();
		return *this;
	}
	Transform t = Transform();
	t.position = parent->position + _position;
	t.scale = parent->scale * _scale;
	t.rotation = _rotation * parent->rotation;
	t.parent = parent->parent;
	return t.computeTransform();
}

void Transform::updateNormals() {
	glm::mat4 m = glm::rotate(_rotAngle, _rotAxis);
	_forward = (glm::vec3)(m * glm::vec4(0, 0, 1, 1));
	_up = (glm::vec3)(m * glm::vec4(0, 1, 0, 1));
	_right = glm::cross(_forward, _up);
}

glm::vec3 Transform::forward() const { return _forward; }
glm::vec3 Transform::up() const { return _up; }
glm::vec3 Transform::right() const { return _right; }

void Transform::updateRot() {
	_rotAngle = glm::angle(_rotation);
	_rotAxis = glm::axis(_rotation);
	updateNormals();
}

//remember for quats, glm applies rotations through the glm::rotate(quat,rad,axis) function, and the return value is the rotated quat
void Transform::rotate(float x, float y, float z) {
	if(x)
		_rotation = glm::rotate(_rotation, x, glm::vec3(1, 0, 0));
	if(y)
		_rotation = glm::rotate(_rotation, y, glm::vec3(0, 1, 0));
	if(z)
		_rotation = glm::rotate(_rotation, z, glm::vec3(0, 0, 1));
	updateRot();
}

void Transform::rotate(glm::vec3 v) {
	if (v.x)
		_rotation = glm::rotate(_rotation, v.x, glm::vec3(1, 0, 0));
	if (v.y)
		_rotation = glm::rotate(_rotation, v.y, glm::vec3(0, 1, 0));
	if (v.z)
		_rotation = glm::rotate(_rotation, v.z, glm::vec3(0, 0, 1));
	updateRot();
}

void Transform::rotate(float theta, glm::vec3 axis) {
	if (theta)
		_rotation = glm::rotate(_rotation, theta, axis);
	updateRot();
}

glm::vec3 Transform::getTransformed(glm::vec3 v)
{
	Transform t = computeTransform();
	glm::mat4 translate = glm::translate(t.position);
	glm::mat4 rot = glm::rotate(t.rotAngle, t.rotAxis);
	glm::mat4 scale = glm::scale(t.scale);
	return (glm::vec3)(translate * scale * rot * glm::vec4(v, 1));
}
