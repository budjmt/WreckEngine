#include "Collider.h"


Collider::Collider(void)
	: type(etype)
{
	cdims = glm::vec3(0,0,0);
	cradius = 0;
}

Collider::Collider(Transform* t, glm::vec3 d)
	: type(etype)
{
	dims(d);
	ctrans = t;
	etype = ColliderType::BOX;
	genNormals();
}

Collider::Collider(const Collider& other)
	: type(etype)
{
	dims(other.cdims);
}

Collider::~Collider(void)
{
}

Transform* Collider::transform() { return ctrans; }
glm::vec3 Collider::dims() { return cdims; } void Collider::dims(glm::vec3 v) { cdims = v; cradius = glm::max(glm::max(cdims.x, cdims.y), cdims.z); }
float Collider::radius() const { return cradius; }

bool Collider::intersects2D(Collider other) 
{
	//circle collision optimization
	if ((ctrans->position - other.transform()->position).length() > cradius + other.radius())// || (type == CIRCLE && other.type == CIRCLE))
		return false;
	//separating axis theorem
	std::vector<glm::vec3> axes = getAxes(other);
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

SupportPoint Collider::getSupportPoint(glm::vec3 dir) {
	SupportPoint support{ verts[0], verts[0].dot(dir) };
	int numVerts = verts.size();
	for (int i = 1; i < numVerts; i++) {
		float proj = glm::dot(verts[i], dir);
		if (proj > support.proj) {
			support.point = verts[i];
			support.proj = proj;
		}
	}
	return support;
}

//returns the normal and vertex with the greatest penetration,
//this can be the minimum axis of penetration (confusingly enough)
//the reasoning is that if the value is negative, there is penetration,
//so the greatest NEGATIVE value has the least penetration
//if the value is positive, then there is no penetration i.e. there is a separating axis
Manifold Collider::getAxisMinPen(Collider* other) {
	Manifold axis;
	axis.originator = this;
	axis.pen = -1 / 0.f;
	int numNormals = currNormals.size();
	for (int i = 0; i < numNormals; i++) {
		glm::vec3 norm = currNormals[i];
		SupportPoint support = other->getSupportPoint(-norm);
		glm::vec3 vert = verts[i];//no

		float pen = glm::dot(norm, support.point - vert);
		if (pen > axis.pen) {
			axis.axis = norm;
			axis.colPoint = vert;
			axis.pen = pen;
		}
	}
	return axis;
}

Manifold Collider::intersects(Collider other) {
	//quick circle collision optimization
	Manifold manifold;
	glm::vec3 d = center - other.center;
	float distSq = d.x * d.x + d.y * d.y + d.z * d.z;
	float rad = cradius + other.radius();
	if (distSq > rad * rad)
		return manifold;//originator will be null, i.e. there's no collision

	//separating axis theorem 2: electric boogaloo
	//this contains the collision data

	Manifold minAxis = getAxisMinPen(&other);
	if (minAxis.pen > 0)
		return manifold;

	Manifold otherMinAxis = other.getAxisMinPen(this);
	//this may be unnecessary
	if (otherMinAxis.pen > 0)
		return manifold;

	manifold = (minAxis.pen > otherMinAxis.pen) ? minAxis : otherMinAxis;
	//console.log(manifold.pen);
	return manifold;
}

//the code below will need updating when the system is updated for 3D
/*void Collider::setCorners(std::vector<glm::vec3> c) {
	corners = c;
}*/

void Collider::genNormals() {
	switch (etype) {
	case ColliderType::SPHERE:
		break;
	case ColliderType::BOX:
		normals.push_back(glm::vec3(1, 0, 0));
		normals.push_back(glm::vec3(0, 1, 0));
		normals.push_back(glm::vec3(0, 0, 1));
		edges.push_back(glm::vec3(1, 0, 0));
		edges.push_back(glm::vec3(0, 1, 0));
		edges.push_back(glm::vec3(0, 0, 1));
		break;
	case ColliderType::MESH:
		//stating here and now that this is not robust enough for detailed collision meshes
		//so they should be as low VERTEX count as possible
		//since I haven't optimized edges yet
		//...
		//generate the face normals from the mesh's normals
		int numNormals = mesh->normals().size();
		std::vector<glm::vec3> meshNormals = mesh->normals();
		for (int i = 0; i < numNormals; i++)
			addUniqueAxis(normals, meshNormals[i]);
		//also set up the unique edges
		//thiiiis is super slow
		int numFaces = mesh->faces().verts.size();
		std::vector<glm::vec3> meshVerts = mesh->verts();
		std::vector<GLuint> faceVerts = mesh->faces().verts;
		for (int i = 0; i < numFaces; i += 3) {
			glm::vec3 p1, p2, p3;
			p1 = meshVerts[faceVerts[i]];
			p2 = meshVerts[faceVerts[i + 1]];
			p3 = meshVerts[faceVerts[i + 2]];
			addUniqueAxis(edges, p1 - p2);
			addUniqueAxis(edges, p2 - p3);
			addUniqueAxis(edges, p3 - p1);
		}
		break;
	}
}

bool Collider::fuzzySameDir(glm::vec3 v1, glm::vec3 v2) {
	if (v1 == v2)
		return true;
	float propx = v2.x / v1.x;
	float eps = glm::epsilon<float>();
	return fabs((v2.y / v1.y - propx) / propx) < eps && fabs(propx - v2.z / v1.z) < eps;
}

void Collider::addUniqueAxis(std::vector<glm::vec3>& axes, glm::vec3 axis) {
	//this really should be optimized, but I'll worry about that later
	int numAxes = axes.size();
	for (int i = 0; i < numAxes - 1; i++) {
		glm::vec3 v = axes[i];
		if (fuzzySameDir(v,axis))
			return;
		//the vector is sorted in ascending order, by x, then y, then z, which is why this works
		bool greater = v.x < axis.x && v.y < axis.y && v.z < axis.z;
		if (greater) {
			axes.insert(axes.begin() + i, axis);
			return;
		}
	}
	//we know for certain it's not in there now, and also is "greater" than all the other vec3s
	//our worst case right now
	axes.push_back(axis);
}

void Collider::updateNormals() {
	//normals.empty();
	switch (type) {
	case ColliderType::SPHERE:
		break;
	/*case RECT:
		for (unsigned int i = 0; i < corners.size(); i++) {
			glm::vec3 norm = corners[i] - corners[(i + 1) % corners.size()];
			norm = glm::vec3(-norm.y, norm.x, 0);
			norm = glm::normalize(norm);
			normals.push_back(norm);
		}
		break;*/
	case ColliderType::BOX:
		glm::mat4 rot = glm::rotate(ctrans->rotAngle,ctrans->rotAxis);
		for (unsigned int i = 0; i < normals.size(); i++) {
			currNormals[i] = (glm::vec3)(rot * normals[i]);
		}
	}
}

const std::vector<glm::vec3>& Collider::getNormals() const {
	return currNormals;
}

void Collider::getMaxMin(glm::vec3 axis, float* maxmin) {
	maxmin[0] = glm::dot(mesh->verts()[0], axis);
	maxmin[1] = 1;
	int numVerts = mesh->verts().size();
	for (int i = 1; i < numVerts; i++) {
		float proj = glm::dot(mesh->verts()[i], axis);
		if (maxmin[1] > proj)
			maxmin[1] = proj;
		if (proj > maxmin[0])
			maxmin[0] = proj;
	}
}

std::vector<glm::vec3> Collider::getAxes(const Collider& other) {
	std::vector<glm::vec3> combNormals = currNormals
						 , otherNormals = other.getNormals();
	combNormals.insert(combNormals.end(), otherNormals.begin(), otherNormals.end());
	return combNormals;
}