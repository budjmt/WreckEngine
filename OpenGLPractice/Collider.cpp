#include "Collider.h"


Collider::Collider(void)
	: pos(cpos), dims(cdims), type(etype)
{
	pos = glm::vec3(0,0,0);
	dims = glm::vec3(0,0,0);
}

Collider::Collider(glm::vec3 p, glm::vec3 b)
	: pos(cpos), dims(cdims), type(etype)
{
	dims = b;
	pos = p - dims * 0.5f;
}

Collider::Collider(const Collider& other)
	: pos(cpos), dims(cdims), type(etype)
{
	pos = other.pos;
	dims = other.dims;
}

Collider::~Collider(void)
{
}

bool Collider::intersects(Collider other) 
{
	//circle collision optimization
	if ((pos - other.pos).length() > glm::max(dims.x, dims.y) + glm::max(other.dims.x, other.dims.y) || (type == CIRCLE && other.type == CIRCLE))
		return false;
	//separating axis theorem
	std::vector<glm::vec3> axes = getNormals(other);
	float projs[2], otherProjs[2];
	for (glm::vec3 axis : axes) {
		getMaxMin(axis,projs);
		other.getMaxMin(axis,otherProjs);
		if (projs[0] < otherProjs[1] || otherProjs[0] < projs[1]) {
			return false;
		}
	}
	return true;
}

//the code below will need updating when the system is updated for 3D
void Collider::setCorners(std::vector<glm::vec3> c) {
	corners = c;
}

void Collider::updateNormals() {
	normals.empty();
	switch (type) {
	case CIRCLE:
		break;
	case RECT:
		for (unsigned int i = 0; i < corners.size(); i++) {
			glm::vec3 norm = corners[i] - corners[(i + 1) % corners.size()];
			norm = glm::vec3(-norm.y, norm.x, 0);
			norm = glm::normalize(norm);
			normals.push_back(norm);
		}
		break;
	}
}

void Collider::getMaxMin(glm::vec3 axis, float* maxmin) {
	maxmin[0] = glm::dot(corners[0], axis);
	maxmin[1] = 1;
	for (unsigned int i = 1; i < corners.size(); i++) {
		float proj = glm::dot(corners[i], axis);
		if (maxmin[1] > proj)
			maxmin[1] = proj;
		if (proj > maxmin[0])
			maxmin[0] = proj;
	}
}

std::vector<glm::vec3> Collider::getNormals(const Collider& other) {
	std::vector<glm::vec3> combNormals = normals;
	for (glm::vec3 normal : other.normals)
		combNormals.push_back(normal);
	return combNormals;
}