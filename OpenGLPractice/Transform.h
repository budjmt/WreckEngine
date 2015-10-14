#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

class Transform
{
public:
	Transform();
	Transform(const Transform& other);
	~Transform();
	Transform& operator=(const Transform& other);
	glm::vec3& position;
	glm::vec3& scale;
	glm::fquat& rotation;
	glm::vec3& rotAxis;
	float& rotAngle;
	glm::vec3& forward, &up, &right;
	Transform* parent;
	Transform computeTransform();
	void updateNormals();//this will replace the ones below
	glm::vec3 getForward();
	glm::vec3 getUp();
	glm::vec3 getRight();
	void updateRot();
	void rotate(float x, float y, float z);
	void rotate(glm::vec3 v);
	void rotate(float theta, glm::vec3 axis);
private:
	glm::vec3 tposition;
	glm::vec3 tscale;
	glm::fquat trotation;
	glm::vec3 trotAxis;
	float trotAngle;
	glm::vec3 tforward, tup, tright;
};

