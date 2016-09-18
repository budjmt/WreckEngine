#pragma once

#include <unordered_set>

#include "MarchMath.h"
#include "smart_ptr.h"

class Transform
{
public:
	Transform();
	//Transform(const Transform& other);
	//Transform(Transform&& other) = default;
	//Transform& operator=(const Transform& other);
	//Transform& operator=(Transform&& other) = default;

	Transform* clone();

	bool dirty;
	void makeDirty();
	vec3& position(); void position(vec3 v);
	vec3& scale(); void scale(vec3 v);
	quat& rotation(); void rotation(quat q);
	vec3 rotAxis() const; float rotAngle() const;

	Transform* parent(); void parent(Transform* p);
	std::unordered_set<Transform*> children;
	Transform* getComputed();

	void setBaseDirections(vec3 t_forward, vec3 t_up);
	void updateDirections();
	vec3 forward() const;
	vec3 up() const;
	vec3 right() const;
	void updateRot();
	void rotate(float x, float y, float z);
	void rotate(vec3 v);
	void rotate(float theta, vec3 axis);

	vec3 getTransformed(vec3 v);
private:
	vec3 _position, _scale = vec3(1);
	quat _rotation; vec3 _rotAxis; float _rotAngle;
	vec3 base_forward = vec3(0, 0, 1), _forward, base_up = vec3(0, 1, 0), _up, _right;

	alloc<Transform> computed = alloc<Transform>(nullptr);
	Transform* _parent = nullptr;
	Transform* computeTransform();

	//typedef Transform*(Transform::*ComputeTransformFunc)();
	Transform* noCompute();
	Transform* firstCompute();
	Transform* allocatedCompute();
	//ComputeTransformFunc compute = &Transform::noCompute;
};