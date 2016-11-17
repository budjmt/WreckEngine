#pragma once

#include <unordered_set>
#include <malloc.h>

#include "MarchMath.h"
#include "smart_ptr.h"
#include "property.h"

class Transform
{
public:
	Transform();

	struct alignas(16) mat_cache {
		mat4 translate, rotate, scale, world;
		virtual ~mat_cache() = default;
		void* operator new(size_t i) { return _aligned_malloc(i, 16); }
		void operator delete(void* p) { _aligned_free(p); }
	};

	void makeDirty() const;

	Transform* parent() const; void parent(Transform* p);
	std::unordered_set<Transform*> children;
	const Transform* getComputed() const;
	mat_cache* getMats() const;

	void setBaseDirections(const vec3 t_forward, const vec3 t_up);
	void rotate(const float x, const float y, const float z);
	void rotate(const vec3 v);
	void rotate(const float theta, const vec3 axis);

	vec3 getTransformed(const vec3 v) const;
private: 
	PROP_GS (private, Transform, vec3, position, { return _position; }, { makeDirty(); return _position = value; });
	PROP_GS (private, Transform, vec3, scale,    { return _scale;    }, { makeDirty(); return _scale = value; }) = vec3(1);
	PROP_GS(private, Transform, quat, rotation, { return _rotation; }, { makeDirty(); _rotation = value; updateRot(); return _rotation; });
	ACCS_G  (private, vec3, rotAxis);
	ACCS_G  (private, float, rotAngle);

	vec3 base_forward = vec3(0, 0, 1)
	   , base_up      = vec3(0, 1, 0);
	ACCS_G (private, vec3, forward);
	ACCS_G (private, vec3, up);
	ACCS_G (private, vec3, right);

	// computes are const, so these must be mutable
	mutable bool dirtyComp, dirtyMats;
	mutable alloc<Transform> computed = alloc<Transform>(nullptr);
	mutable alloc<mat_cache> mats = alloc<mat_cache>(nullptr);
	
	Transform* _parent = nullptr;
	Transform* computeTransform() const;

	void updateDirections();
	void updateRot();
	void updateMats() const;
};