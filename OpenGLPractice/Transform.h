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
	glm::vec3 forward() const;
	glm::vec3 up() const;
	glm::vec3 right() const;
	void updateRot();
	void rotate(float x, float y, float z);
	void rotate(glm::vec3 v);
	void rotate(float theta, glm::vec3 axis);

	glm::vec3 getTransformed(glm::vec3 v);
private:
	glm::vec3 _position;
	glm::vec3 _scale;
	glm::fquat _rotation;
	glm::vec3 _rotAxis;
	float _rotAngle;
	glm::vec3 _forward, _up, _right;
};

