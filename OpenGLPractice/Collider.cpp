#include "Collider.h"


Collider::Collider(void)
	: type(_type)
{
	_dims = glm::vec3(0,0,0);
	_radius = 0;
}

Collider::Collider(Transform* t, glm::vec3 d)
	: type(_type)
{
	dims(d);
	_transform = t;
	_type = ColliderType::BOX;
	genNormals();
}

Collider::Collider(const Collider& other)
	: type(_type)
{
	dims(other._dims);
}

Collider::~Collider(void)
{
}

Transform* Collider::transform() { return _transform; }
glm::vec3 Collider::framePos() { return _framePos; }

glm::vec3 Collider::dims() { return _dims; } void Collider::dims(glm::vec3 v) { _dims = v; _radius = glm::max(glm::max(_dims.x, _dims.y), _dims.z); }
float Collider::radius() const { return _radius; }

bool Collider::intersects2D(Collider other) 
{
	//circle collision optimization
	if ((_transform->position - other.transform()->position).length() > _radius + other.radius())// || (type == CIRCLE && other.type == CIRCLE))
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
	auto verts = mesh->verts();
	SupportPoint support{ verts[0], glm::dot(verts[0], dir) };
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

Manifold Collider::getAxisMinPen(Collider* other) {
	return getAxisMinPen(other, currNormals);
}

//returns the normal and vertex with the greatest penetration,
//this can be the minimum axis of penetration (confusingly enough)
//the reasoning is that if the value is negative, there is penetration,
//so the greatest NEGATIVE value has the least penetration
//if the value is positive, then there is no penetration i.e. there is a separating axis
Manifold Collider::getAxisMinPen(Collider* other, std::vector<glm::vec3>& axes) {
	Manifold axis;
	axis.originator = this;
	axis.pen = -1 / 0.f;
	int numAxes = axes.size();
	for (int i = 0; i < numAxes; i++) {
		glm::vec3 norm = axes[i];
		SupportPoint support = other->getSupportPoint(-norm);
		glm::vec3 vert = verts[i];//no, this needs to be a vertex that goes with this normal

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
	glm::vec3 d = _framePos - other.framePos();//I'm going to ignore displaced colliders for now
	float distSq = d.x * d.x + d.y * d.y + d.z * d.z;
	float rad = _radius + other.radius();
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
	switch (_type) {
	case ColliderType::SPHERE:
		break;
	case ColliderType::BOX:
		normals.push_back(0);//need to define some kind of vertex and normal array for box colliders
		normals.push_back(1);
		normals.push_back(2);
		break;
	case ColliderType::MESH:
		//generate the face normals from the mesh's normals
		int numNormals = mesh->normals().size();
		for (int i = 0; i < numNormals; i++)
			addUniqueAxis(normals, i);
		break;
	}
}

void Collider::genGaussMap() {
	switch (_type) {
	case ColliderType::SPHERE:
		break;
	case ColliderType::BOX:
		gauss.adjacencies[0] = { Adj{ 1, {,} }, Adj{ 2, {,} } };
		gauss.adjacencies[1] = { Adj{ 2, {,} } };
		break;
	case ColliderType::MESH:
		//set up the edge associations
		std::vector<GLuint>& faceVerts = mesh->faces().verts;
		int numFaces = faceVerts.size();
		for (int i = 0; i < numFaces; i += 3) {
			for (int j = i + 3; j < numFaces; j++) {
				Adj a;
				a.edge[0] = -1; a.edge[1] = -1;
				bool added = false;
				for (int p1 = 0; !added && p1 < 3; p1++) {
					for (int p2 = 0; p2 < 3; p2++) {
						//checks if a pair of vertices match
						if (faceVerts[i + p1] == faceVerts[j + p2]) {
							//if a match hasn't been found yet, just record it
							if (a.edge[0] < 0) 
								a.edge[0] = faceVerts[i + p1];
							//otherwise, record it, set the normal, push it back, and end the loop
							else {
								a.edge[1] = faceVerts[i + p1]; a.normal = j % 3;//I just realized this and the way I normally do normals is now fucked
								//essentially, I need an array of face normals now which the indexes correspond to this... hmm
								//a map? probably. map face normals (edge cross product, different from point normals) to index values?
								//I really just need a way to connect the adjacencies back to the face normals
								//for the gauss map, I'll be iterating over the face normals
								//so the face normals are key
								//and the info I need is the associations...
								//which should also point to face normals
								//the way I'm deriving this is from the points on the faces
								//from these I can derive a face normal, i.e. the key
								//I can also derive a face normal from the other face
								gauss.adjacencies[i % 3].push_back(a);
								added = true;
							}
							break;
						}
					}
				}
			}
		}
		break;
	}
}

bool Collider::fuzzySameDir(glm::vec3 v1, glm::vec3 v2) {
	if (v1 == v2)
		return true;
	float propx = v2.x / v1.x;
	float eps = glm::epsilon<float>();
	return fabs((v2.y / v1.y - propx) / propx) < eps && fabs((v2.z / v1.z - propx) / propx) < eps;
}

void Collider::addUniqueAxis(std::vector<int>& axes, int aIndex) {
	//this really should be optimized, but I'll worry about that later
	std::vector<glm::vec3> meshNormals = mesh->normals();
	glm::vec3 axis = meshNormals[aIndex];
	//axis = glm::normalize(axis);//hmm
	int numAxes = axes.size();
	for (int i = 0; i < numAxes - 1; i++) {
		glm::vec3 v = meshNormals[axes[i]];
		if (fuzzySameDir(v,axis))
			return;
		//the vector is sorted in ascending order, by x, then y, then z, which is why this works
		bool greater = v.x < axis.x && v.y < axis.y && v.z < axis.z;
		if (greater) {
			axes.insert(axes.begin() + i, aIndex);
			return;
		}
	}
	//we know for certain it's not in there now, and also is "greater" than all the other vec3s
	//our worst case right now
	axes.push_back(aIndex);
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
		glm::mat4 rot = glm::rotate(_transform->rotAngle, _transform->rotAxis);
		int numNormals = normals.size();
		for (int i = 0; i < numNormals; i++) {
			currNormals[i] = (glm::vec3)(rot * glm::vec4(normals[i], 1));//this is probably slow
		}
		int numEdges = edges.size();
		for (int i = 0; i < numEdges; i++) {
			currEdges[i] = (glm::vec3)(rot * glm::vec4(edges[i], 1));//this is probably slow
		}
	}
}

void Collider::update() {
	_framePos = _transform->computeTransform().position;
	updateNormals();
}

const std::vector<glm::vec3>& Collider::getNormals() const { return currNormals; }
const std::vector<glm::vec3>& Collider::getEdges() const { return currEdges; }

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
	
	std::vector<glm::vec3> edgeNormals;
	int numEdges = currEdges.size();
	auto otherEdges = other.getEdges();
	int othernumEdges = otherEdges.size();
	for (int i = 0; i < numEdges; i++) {
		for (int j = 0; j < othernumEdges; j++) {
			addUniqueAxis(edgeNormals, glm::cross(currEdges[i], otherEdges[j]));
		}
	}

	combNormals.insert(combNormals.end(), edgeNormals.begin(), edgeNormals.end());
	return combNormals;
}

void Collider::overlayGaussMaps(Collider& other, std::vector<glm::vec3>& edges) {
	int numNormals = currNormals.size();
	auto otherNormals = other.getNormals();
	int othernumNormals = otherNormals.size();
	for (int i = 0; i < numNormals; i++) {
		for (int j = 0; j < othernumNormals; j++) {

		}
	}
}