#pragma once

#include "glm/glm.hpp"

class Quaternion
{
public:
	Quaternion();
	Quaternion(float qx, float qy, float qz, float qw);
	~Quaternion();
	Quaternion& operator*(const Quaternion& other);
	void setRotation(float rot, glm::vec3 axis);
	Quaternion getInverse();
private:
	float x, y, z, w;
};

