#include "Transform.h"

Transform::Transform() { updateRot(); }
Transform* Transform::clone() { return new Transform(*this); }

void Transform::makeDirty() {
	dirtyComp = dirtyMats = true;
	for (auto child : children)
		child->makeDirty();
}

Transform* Transform::parent() { return _parent; }
void Transform::parent(Transform* p) {
	if (p == _parent) return;
	// current parent must remove us as a child
	if (_parent) { _parent->children.erase(this); }
	else { mats.release(); }
	// new parent must add us, or if there is none we no longer need to compute a transform
	if (p) { p->children.insert(this); }
	else { computed.release(); }
	makeDirty();
	_parent = p;
}

TransformMats* Transform::getMats() {
	auto t = getComputed();
	if (!t->mats) { t->mats.reset(new TransformMats); goto DIRTY; }
	if (dirtyMats) {
	DIRTY: 
		dirtyMats = false;
		t->updateMats();
	}
	return t->mats.get();
}

void Transform::updateMats() {
	auto t = glm::translate(_position);
	auto r = glm::rotate(_rotAngle, _rotAxis);
	auto s = glm::scale(_scale);
	mats->translate = t;
	mats->rotate = r;
	mats->scale = s;
	mats->world = t * r * s;
}

Transform* Transform::getComputed() {
	//if there's no parent, this is already an accurate transform
	if (!_parent) return this;
	//if there is no previously computed transform, allocate one so we can compute it
	else if (!computed) computed.reset(new Transform);
	//if there is a previously computed transform and we haven't made any "dirtying" changes since computing it
	else if (!dirtyComp) return computed.get();
	//[re]compute the transform
	dirtyComp = false;
	return computeTransform();
}

Transform* Transform::computeTransform() {
	auto p = _parent->getComputed();
	computed->_position = _position + p->_position;
	computed->_scale    = _scale * p->_scale;
	computed->_rotation = _rotation * p->_rotation;
	computed->updateRot();
	return computed.get();
}

void Transform::setBaseDirections(vec3 t_forward, vec3 t_up) {
	base_forward = t_forward;
	base_up = t_up;
	updateDirections();
}

void Transform::updateDirections() {
	auto r = glm::rotate(_rotAngle, _rotAxis);
	_forward = (vec3)(r * vec4(base_forward, 1));
	_up      = (vec3)(r * vec4(base_up, 1));
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
	if (x) { _rotation = quat::rotate(_rotation, x, vec3(1, 0, 0)); makeDirty(); }
	if (y) { _rotation = quat::rotate(_rotation, y, vec3(0, 1, 0)); makeDirty(); }
	if (z) { _rotation = quat::rotate(_rotation, z, vec3(0, 0, 1)); makeDirty(); }
	updateRot();
}

void Transform::rotate(float theta, vec3 axis) {
	if (theta) { _rotation = quat::rotate(_rotation, theta, axis); makeDirty(); }
	updateRot();
}

vec3 Transform::getTransformed(vec3 v)
{
	return (vec3)(getMats()->world * vec4(v, 1));
}