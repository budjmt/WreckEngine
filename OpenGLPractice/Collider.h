#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"
#include <vector>

enum ColliderType {
	CIRCLE,
	RECT,
	TRIANGLE
};

class Collider
{
public:
	Collider(void);
	Collider(glm::vec3 p, glm::vec3 b);
	Collider(const Collider& other);
	~Collider(void);
	glm::vec3& pos;
	glm::vec3& dims;
	ColliderType& type;
	bool intersects(Collider other);

	void setCorners(std::vector<glm::vec3> c);
	void updateNormals();
	void getMaxMin(glm::vec3 axis,float* maxmin);
	std::vector<glm::vec3> getNormals(const Collider& other);
private:
	glm::vec3 cpos;
	glm::vec3 cdims;
	ColliderType etype;

	std::vector<glm::vec3> corners, normals;

};

