#pragma once

#include "glm/glm.hpp"

class Transform
{
public:
	Transform();
	~Transform();
	glm::vec3& position;
	glm::vec3& scale;
	glm::vec3& rotAxis;
	float& rotation;
	Transform computeTransform();
private:
	glm::vec3 tpos;
	glm::vec3 tscale;
	glm::vec3 trotAxis;
	float trot;
};

