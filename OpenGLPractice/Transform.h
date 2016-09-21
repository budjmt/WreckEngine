#pragma once

#include <unordered_set>

#include "MarchMath.h"
#include "smart_ptr.h"
#include "property.h"

class Transform
{
public:
	Transform();
	Transform* clone();

	bool dirty;
	void makeDirty();

	Transform* parent(); void parent(Transform* p);
	std::unordered_set<Transform*> children;
	Transform* getComputed();

	void setBaseDirections(vec3 t_forward, vec3 t_up);
	void updateDirections();
	void updateRot();
	void rotate(float x, float y, float z);
	void rotate(vec3 v);
	void rotate(float theta, vec3 axis);

	vec3 getTransformed(vec3 v);
private: 
	PROP_GS (Transform, vec3, position, { return _position; }, { makeDirty(); return _position = value; });
	PROP_GS (Transform, vec3, scale,    { return _scale;    }, { makeDirty(); return _scale = value; }) = vec3(1);
	PROP_GS (Transform, quat, rotation, { return _rotation; }, { makeDirty(); return _rotation = value; });
	ACCS_G  (vec3, rotAxis);
	ACCS_G  (float, rotAngle);

	vec3 base_forward = vec3(0, 0, 1), base_up = vec3(0, 1, 0);
	ACCS_G (vec3, forward);
	ACCS_G (vec3, up);
	ACCS_G (vec3, right);

	alloc<Transform> computed = alloc<Transform>(nullptr);
	Transform* _parent = nullptr;
	Transform* computeTransform();
};