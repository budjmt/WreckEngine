#include "Transform.h"

Transform::Transform() { updateRot(); }
Transform* Transform::clone() { return new Transform(*this); }

void Transform::makeDirty() {
	dirty = true;
	for (auto child : children)
		child->dirty = true;
}

Transform* Transform::parent() { return _parent; }
void Transform::parent(Transform* p) {
	if (p == _parent) return;
	// current parent must remove us as a child
	if (_parent) { _parent->children.erase(this); }
	// new parent must add us, or if there is none we no longer need to compute a transform
	if (p) { p->children.insert(this); }
	else   { computed.release(); }
	makeDirty();
	_parent = p;
}

Transform* Transform::getComputed() {
	//if there's no parent, this is already an accurate transform
	if (!_parent) return this;
	//if there is no previously computed transform, allocate one so we can compute it
	else if (!computed) computed.reset(new Transform);
	//if there is a previously computed transform and we haven't made any "dirtying" changes since computing it
	else if (!dirty) return computed.get();
	//[re]compute the transform
	dirty = false;
	return computeTransform();
}

Transform* Transform::computeTransform() {
	if (!_parent) {
		updateRot();
		return this;
	}
	Transform t;
	t._position += _parent->_position;
	t._scale    *= _parent->_scale;
	t._rotation  = t._rotation * _parent->_rotation;
	t._parent    = _parent->_parent;
	
	*computed = *t.computeTransform();
	return computed.get();
}

void Transform::setBaseDirections(vec3 t_forward, vec3 t_up) {
	base_forward = t_forward;
	base_up = t_up;
	updateDirections();
}

void Transform::updateDirections() {
	auto m = glm::rotate(_rotAngle, _rotAxis);
	_forward = (vec3)(m * vec4(base_forward, 1));
	_up      = (vec3)(m * vec4(base_up, 1));
	_right   = glm::cross(_up, _forward);
}

void Transform::updateRot() {
	_rotAngle = _rotation.theta();
	_rotAxis  = _rotation.axis();
	updateDirections();
}

//quats are rotated through the rotate function here
void Transform::rotate(vec3 v) { rotate(v.x, v.y, v.z); }
void Transform::rotate(float x, float y, float z) {
	if (x) _rotation = quat::rotate(_rotation, x, vec3(1, 0, 0));
	if (y) _rotation = quat::rotate(_rotation, y, vec3(0, 1, 0));
	if (z) _rotation = quat::rotate(_rotation, z, vec3(0, 0, 1));
	updateRot();
}

void Transform::rotate(float theta, vec3 axis) {
	if (theta) _rotation = quat::rotate(_rotation, theta, axis);
	updateRot();
}

vec3 Transform::getTransformed(vec3 v)
{
	auto t = getComputed();
	auto translate = glm::translate(t->_position);
	auto rotate    = glm::rotate(t->_rotAngle, t->_rotAxis);
	auto scale     = glm::scale(t->_scale);
	return (vec3)(translate * (rotate * (scale * vec4(v, 1))));
}