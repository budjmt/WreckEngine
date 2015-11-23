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
	Transform* parent;
	Transform computeTransform();
	void updateNormals();//this will replace the ones below
	glm::vec3 forward();
	glm::vec3 up();
	glm::vec3 right();
	void updateRot();
	void rotate(float x, float y, float z);
	void rotate(glm::vec3 v);
	void rotate(float theta, glm::vec3 axis);

	glm::vec3 getTransformed(glm::vec3 v);
private:
	glm::vec3 tposition;
	glm::vec3 tscale;
	glm::fquat trotation;
	glm::vec3 trotAxis;
	float trotAngle;
	glm::vec3 tforward, tup, tright;
};

