#pragma once

#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/constants.hpp"

#include <vector>
#include "Transform.h"
#include "Mesh.h"

//2D
enum ColliderType {
	//CIRCLE,
	//RECT,
	//TRIANGLE
	SPHERE,
	BOX,
	MESH
};

struct SupportPoint {
	glm::vec3 point;
	float proj;
};

struct Manifold {
	Collider* originator;
	glm::vec3 axis, colPoint;
	float pen;
};

class Collider
{
public:
	Collider(void);
	Collider(Transform* t, glm::vec3 b);
	Collider(const Collider& other);
	~Collider(void);
	ColliderType& type;
	Transform* transform();
	glm::vec3 dims(); void dims(glm::vec3 v);

	bool intersects2D(Collider other);
	Manifold intersects(Collider other);

	//void setCorners(std::vector<glm::vec3> c);
	void genNormals();
	void updateNormals();
	const std::vector<glm::vec3>& getNormals() const;

	void getMaxMin(glm::vec3 axis,float* maxmin);

	SupportPoint getSupportPoint(glm::vec3 dir);
	Manifold getAxisMinPen(Collider* other);

	std::vector<glm::vec3> getAxes(const Collider& other);

	void addUniqueAxis(std::vector<glm::vec3>& axes, glm::vec3 axis);
private:
	Transform* ctrans;
	glm::vec3 cdims;
	ColliderType etype;

	std::vector<glm::vec3> normals, edges, currNormals;//these are vec3s to avoid constant typecasting, and b/c cross product doesn't work for 4d vectors
	Mesh* mesh;
};

