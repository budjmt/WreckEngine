#include "Transform.h"

Transform::Transform() { updateRot(); }

void Transform::makeDirty() const {
    dirtyComp = dirtyMats = true;
    for (auto child : children)
        child->makeDirty();
}

Transform* Transform::parent() const { return _parent; }
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

Transform::mat_cache* Transform::getMats() const {
    auto t = getComputed();
    if (!t->mats) { t->mats.reset(new mat_cache); goto DIRTY; }
    if (dirtyMats) {
    DIRTY:
        dirtyMats = false;
        t->updateMats();
    }
    return t->mats.get();
}

void Transform::updateMats() const {
    const auto t = glm::translate(_position);
    const auto r = glm::rotate(_rotAngle, _rotAxis);
    const auto s = glm::scale(_scale);
    mats->translate = t;
    mats->rotate = r;
    mats->scale = s;
    mats->world = t * r * s;
}

// WARNING: this is guaranteed to be safe across threads, but the biggest danger is in a single thread
// If the pointer is copied, then the transform is changed,
// and then another call to get the pointer is made IN THE SAME THREAD before the old copy is destroyed
// the program will deadlock because the shared lock will never be released
// This can be avoided by:
//   a) not calling getComputed() multiple times in the same (broad) scope (difficult)
//   b) getting individual values from each getComputed() call (easy, but sometimes inefficient)
//   c) declaring copy variables, then assigning them in a dedicated scope with a copy of the pointer (ugly, but most efficient for multiple values)
Transform::safe_tf_ptr Transform::getComputed() const {
    //if there's no parent, this is already an accurate transform
    if (!_parent) return safe_tf_ptr(this, computedMut.object);
    //if there is no previously computed transform, allocate one so we can compute it
    else if (!computed) computed.reset(new Transform);
    //if there is a previously computed transform and we haven't made any "dirtying" changes since computing it
    else if (!dirtyComp) return safe_tf_ptr(computed.get(), computedMut.object);
    //[re]compute the transform
    dirtyComp = false;
    return safe_tf_ptr(computeTransform(), computedMut.object);
}

Transform* Transform::computeTransform() const {
    {
        std::unique_lock<std::shared_mutex> lock(computedMut.object);
        auto p = _parent->getComputed();
        computed->_position = p->_position + _position;
        computed->_scale = p->_scale * _scale;
        computed->_rotation = p->_rotation * _rotation;
        computed->updateRot();
    }
    return computed.get();
}

void Transform::setBaseDirections(const vec3 t_forward, const vec3 t_up) {
    base_forward = t_forward;
    base_up = t_up;
    updateDirections();
}

void Transform::updateDirections() {
    const auto r = glm::rotate(_rotAngle, _rotAxis);
    _forward = (vec3)(r * vec4(base_forward, 1));
    _up = (vec3)(r * vec4(base_up, 1));
    _right = glm::cross(_up, _forward);
}

void Transform::updateRot() {
    _rotAngle = _rotation.theta();
    _rotAxis = _rotation.axis();
    updateDirections();
}

//quats are rotated through the rotate function here
void Transform::rotate(const vec3 v) { rotate(v.x, v.y, v.z); }
void Transform::rotate(const float x, const float y, const float z) {
    auto dirtied = false;
    if (x) { _rotation = quat::rotate(_rotation, x, vec3(1, 0, 0)); dirtied = true; }
    if (y) { _rotation = quat::rotate(_rotation, y, vec3(0, 1, 0)); dirtied = true; }
    if (z) { _rotation = quat::rotate(_rotation, z, vec3(0, 0, 1)); dirtied = true; }

    if (dirtied) { makeDirty(); updateRot(); }
}

void Transform::rotate(const float theta, const vec3 axis) {
    if (theta) rotation = quat::rotate(_rotation, theta, axis);
}

vec3 Transform::getTransformed(const vec3 v) const {
    return (vec3)(getMats()->world * vec4(v, 1));
}