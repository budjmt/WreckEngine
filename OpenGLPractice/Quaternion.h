#pragma once

#include "glm/glm.hpp"

class Quaternion
{
public:
	Quaternion();
	Quaternion(float w, float qx, float qy, float qz);
	~Quaternion();
	Quaternion& operator*(const Quaternion& other);
	void setRotation(float rot, glm::vec3 axis);
	Quaternion getInverse();
private:
	float w, x, y, z;
};

