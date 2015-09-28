#pragma once

#include "glm/glm.hpp"

class Transform
{
public:
	Transform();
	Transform(const Transform& other);
	~Transform();
	Transform& operator=(const Transform& other);
	glm::vec3& position;
	glm::vec3& scale;
	glm::vec3& rotAxis;
	float& rotation;
	glm::vec3& forward, &up, &right;
	Transform* parent;
	Transform computeTransform();
	void updateNormals();
	glm::vec3 getForward();
	glm::vec3 getUp();
	glm::vec3 getRight();
private:
	glm::vec3 tposition;
	glm::vec3 tscale;
	glm::vec3 trotAxis;
	float trotation;
	glm::vec3 tforward, tup, tright;
};

