#include "Collider.h"
#include <iostream>

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
	//the order is important;
	//edges depend on the gauss map, which depends on the normals
	genNormals();
	genGaussMap();
	genEdges();
	update();
}

Collider::Collider(Mesh* m, Transform* t)
	: type(_type)
{
	mesh = m;
	dims(m->getDims());
	_transform = t;
	_type = ColliderType::MESH;
	//the order is important;
	//edges depend on the gauss map, which depends on the normals
	genNormals();
	genGaussMap();
	genEdges();
	update();
}

Collider::Collider(const Collider& other)
	: type(_type)
{
	dims(other._dims);
}

Collider::~Collider(void)
{
	if (_type == ColliderType::BOX)
		delete mesh;
}

Transform* Collider::transform() { return _transform; }
glm::vec3 Collider::framePos() { return _framePos; }

glm::vec3 Collider::dims() { return _dims; } void Collider::dims(glm::vec3 v) { _dims = v; _radius = glm::max(glm::max(_dims.x, _dims.y), _dims.z); }
float Collider::radius() const { return _radius; }

bool Collider::intersects2D(Collider* other) 
{
	//this method needs to be updated
	//circle collision optimization
	if ((_transform->position - other->transform()->position).length() > _radius + other->radius())// || (type == CIRCLE && other.type == CIRCLE))
		return false;
	//separating axis theorem
	std::vector<glm::vec3> axes = getAxes(other);
	float projs[2], otherProjs[2];
	for (glm::vec3 axis : axes) {
		getMaxMin(axis,projs);
		other->getMaxMin(axis,otherProjs);
		if (projs[0] < otherProjs[1] || otherProjs[0] < projs[1]) {
			return false;
		}
	}
	return true;
}

//gets the vertex of the collider furthest in the direction of dir
SupportPoint Collider::getSupportPoint(glm::vec3 dir) {
	auto verts = mesh->verts();
	Transform trans = _transform->computeTransform();

	SupportPoint support{ trans.getTransformed(verts[0]), 0 };
	support.proj = glm::dot(support.point, dir);
	
	for (int i = 1, numVerts = verts.size(); i < numVerts; i++) {
		glm::vec3 vert = trans.getTransformed(verts[i]);
		float proj = glm::dot(vert, dir);
		if (proj > support.proj) {
			support.point = vert;
			support.proj = proj;
		}
	}
	return support;
}

/*
----------------------------------------------------------------------
	- Returns the normal and vertex on this collider with the greatest penetration into the other collider

	- This is [possibly] the axis of least penetration for the collision (confusingly enough)

	- The reasoning is that if the value is negative, there is penetration, 
	  so the greatest NEGATIVE value has the least penetration 
	  
	- If the value is positive, then there is no penetration i.e. there is a separating axis
----------------------------------------------------------------------
*/
Manifold Collider::getAxisMinPen(Collider* other) {
	Manifold axis;
	axis.originator = this;
	//axis.pen = -FLT_MAX;//keeping this to display initial value

	//int numAxes = uniqueNormals.size();
	int numAxes = currNormals.size();
	auto meshVerts = mesh->verts();
	auto faceVerts = mesh->faces().verts;
	Transform trans = _transform->computeTransform();
	for (int i = 0; axis.pen < 0 && i < numAxes; i++) {
		//glm::vec3 norm = currNormals[uniqueNormals[i]];
		glm::vec3 norm = currNormals[i];
		SupportPoint support = other->getSupportPoint(-norm);
		/*
			this is the main reason unique normals can't be used right now
			if a unique normal is used, it doesn't know which face it's supposed to apply to
			I should be using a plane-centric approach
		*/
		//glm::vec3 vert = trans.getTransformed(meshVerts[faceVerts[uniqueNormals[i] * 3]]);
		glm::vec3 vert = trans.getTransformed(meshVerts[faceVerts[i * 3]]);
		

		float pen = glm::dot(norm, support.point - vert);//point-plane signed distance, negative if penetrating, positive if not
		if (pen > axis.pen) {
			axis.axis = norm;
			axis.colPoint = vert;//need to add clipping the reference plane AND SHIT
			axis.pen = pen;
		}
	}
	return axis;
}

/*
------------------------------------------------------------------------------------------------------------------------------------------------------------------
The principles of using Gauss Maps are as follows:
	- A Gauss Map is defined as the conversion of all the face normals on a body to points on the unit sphere, 
	  and representing adjacencies between the faces the normals refer to as arcs on the sphere.
	
	- By overlaying the gauss maps of two colliders, one can determine which edges on each collider will actually need to be compared for Separating Axis Theorem.
	
	- We determine this by checking for intersecting arcs; recall that arcs represent adjacencies between faces, or in other words edges.
	  Any arcs that intersect indicate that those edges form a face on the Minkowski difference of the two colliders, and therefore their cross product is a normal that must be tested.
	
	- We determine whether two arcs A and B intersect by performing 3 separating axis tests:
		- The vertices P and Q of arc B fall on opposite sides of the plane through arc A
		- The vertices R and S of arc A fall on opposite sides of the plane through arc B
		- Vertices P and S are on the same side of the plane formed by Q and R (this is a hemisphere test, as the tests will fail if they are not on the same side of the sphere)
	  If all 3 tests are passed, then we know the edges form a face on the Minkowski difference

	- I should probably explain the Minkowski difference. Essentially it's the body formed by subtracting all the vertices of one body from each of the vertices from another.
	  It is a useful tool for collision detection, as all the faces of both original bodies are present, with the addition of faces formed by edges that may potentially be separating axes.
	  Additionally, it can be said if the origin is contained within the Minkowski difference, there is overlap between the bodies, as it means at least two points in them are equal.
	  
	- For practical purposes however, it is almost useless, as assembling it is extremely expensive, (you have to form a new one each frame, for each collision check)
	  which is why Gauss maps are valuable; they only need to be assembled once, as all they represent are associations, and current data can be easily referenced.
	  
	- This means that we gain the benefit of not having to find a support point for every possible combination of axes on the two bodies that the Minkowski difference offers
	  while not having to actually assemble one. 
	  
	- Face normals can be tested as they would normally, and should be tested before overlaying the gauss maps, as it requires fewer comparisons. 
	  (though the necessity of the support points may offset this)

	- Incidentally, this test is also referred to as checking for Voronoi region overlap.

	- I recommend http://twvideo01.ubm-us.net/o1/vault/gdc2013/slides/822403Gregorius_Dirk_TheSeparatingAxisTest.pdf if you're interested in reading more on this technique,
	  as this is where most of my research originates.
------------------------------------------------------------------------------------------------------------------------------------------------------------------
*/
EdgeManifold Collider::overlayGaussMaps(Collider* other) {
	EdgeManifold manifold;
	manifold.originator = this;
	//manifold.pen = -FLT_MAX;//initial value

	GaussMap othergauss = other->getGaussMap();
	std::vector<glm::vec3> otherNormals = other->getCurrNormals();

	Transform trans = _transform->computeTransform();
	Transform otherTrans = other->transform()->computeTransform();
	
	for (std::pair<std::string, std::vector<Adj>> pair : gauss.adjacencies) {
		for (int i = 0, numAdj = pair.second.size(); i < numAdj; i++) {
			
			Adj curr = pair.second[i];
			glm::vec3 a = currNormals[curr.f1], b = currNormals[curr.f2];
			
			for (std::pair<std::string, std::vector<Adj>> otherPair : othergauss.adjacencies) {
				for (int j = 0, othernumAdj = otherPair.second.size(); j < othernumAdj; j++) {
					
					//we found a separating axis boys
					if (manifold.pen > 0)
						return manifold;

					Adj otherCurr = otherPair.second[j];
					glm::vec3 c = otherNormals[otherCurr.f1], d = otherNormals[otherCurr.f2];
					
					//checks if the arcs between arc(a,b) and arc(c,d) intersect
					glm::vec3 bxa = glm::cross(b, a);
					float cba = glm::dot(c, bxa)
						, dba = glm::dot(d, bxa);
					
					//if c and d are on different sides of arc ba
					if (cba * dba < 0) {
						
						glm::vec3 dxc = glm::cross(d, c);
						float adc = glm::dot(a, dxc)
							, bdc = glm::dot(b, dxc);
						
						//if a and b are on different sides of arc dc &&
						//if a and d are on the same side of the plane formed by b and c (c . (b x a) * b . (d x c) > 0)
						//(this works because a . (b x c) == c . (b x a) and d . (b x c) == b . (d x c))(scalar triple product identity)
						//[scalar triple product of a,b,c == [abc] = a . (b x c)]
						if (adc * bdc < 0 && cba * bdc > 0) {
							
							glm::vec3 edge = getEdge(curr.edge), otherEdge = other->getEdge(otherCurr.edge);
							//if edges are parallel, we don't care since they don't define a plane
							if (fuzzyParallel(edge, otherEdge))
								continue;

							//check distance from plane defined by edge normal and one vertex on this body's edge
							glm::vec3 edgeNormal = glm::normalize(glm::cross(edge, otherEdge));

							glm::vec3 v1 = getVert(curr.edge[0]),
									  v2 = other->getVert(otherCurr.edge[0]);
							v1 = trans.getTransformed(v1);
							v2 = otherTrans.getTransformed(v2);
							
							edgeNormal *= glm::sign(glm::dot(edgeNormal, v1 - trans.position));//make sure the edge normal is facing outwards from the body
							float pen = glm::dot(edgeNormal, v2 - v1);
							if (pen > manifold.pen) {
								manifold.edgePair[0] = curr;
								manifold.edgePair[1] = otherCurr;
								manifold.pen = pen;
							}
						}
					}//end edge culling

				}
			}//end other gauss loop

		}
	}//end gauss loop
	return manifold;
}

Manifold Collider::intersects(Collider* other) {
	//quick circle collision optimization
	Manifold manifold;
	manifold.originator = nullptr;
	glm::vec3 d = _framePos - other->framePos();//I'm going to ignore displaced colliders for now
	float distSq = d.x * d.x + d.y * d.y + d.z * d.z;
	float rad = _radius + other->radius();
	if (distSq > rad * rad)
		return manifold;//originator will be nullptr, i.e. there's no collision

	//separating axis theorem 2: electric boogaloo
	//this contains the collision data

	Manifold minAxis = getAxisMinPen(other);
	if (minAxis.pen > 0) {
		//std::cout << "This: " << minAxis.pen << "; " << minAxis.axis.x << ", " << minAxis.axis.y << ", " << minAxis.axis.z << std::endl;
		return manifold;
	}

	Manifold otherMinAxis = other->getAxisMinPen(this);
	if (otherMinAxis.pen > 0) {
		//std::cout << "Other: " << otherMinAxis.pen << "; " << otherMinAxis.axis.x << ", " << otherMinAxis.axis.y << ", " << otherMinAxis.axis.z << std::endl;
		return manifold;
	}

	//edges
	EdgeManifold minEdge = overlayGaussMaps(other);
	if (minEdge.pen > 0) {
		glm::vec3 edge1 = getEdge(minEdge.edgePair[0].edge), edge2 = other->getEdge(minEdge.edgePair[1].edge);
		std::cout << "Edge: " << minEdge.pen << "; This: " << edge1.x << ", " << edge1.y << ", " << edge1.z << "; Other: " << edge2.x << ", " << edge2.y << ", " << edge2.z << std::endl;
		return manifold;
	}

	manifold = (minAxis.pen > otherMinAxis.pen) ? minAxis : otherMinAxis;
	if (minEdge.pen > manifold.pen)
		std::cout << "EDGE ";
	//std::cout << manifold.pen << std::endl;
	return manifold;
}

//the code below will need updating when the system is updated for 3D
/*void Collider::setCorners(std::vector<glm::vec3> c) {
	corners = c;
}*/

void Collider::genVerts() {
	if (_type != ColliderType::BOX)
		return;
	std::vector<glm::vec3> verts, norms, uvs;//norms and uvs are empty
	Face faces;
	verts.push_back(glm::vec3( _dims.x,  _dims.y,  _dims.z));
	verts.push_back(glm::vec3(-_dims.x,  _dims.y,  _dims.z));
	verts.push_back(glm::vec3( _dims.x, -_dims.y,  _dims.z));
	verts.push_back(glm::vec3(-_dims.x, -_dims.y,  _dims.z));
	verts.push_back(glm::vec3( _dims.x,  _dims.y, -_dims.z));
	verts.push_back(glm::vec3(-_dims.x,  _dims.y, -_dims.z));
	verts.push_back(glm::vec3( _dims.x, -_dims.y, -_dims.z));
	verts.push_back(glm::vec3(-_dims.x, -_dims.y,- _dims.z));

	faces.verts.push_back(7); faces.verts.push_back(3); faces.verts.push_back(0);
	faces.verts.push_back(7); faces.verts.push_back(4); faces.verts.push_back(0);

	faces.verts.push_back(6); faces.verts.push_back(7); faces.verts.push_back(4);
	faces.verts.push_back(6); faces.verts.push_back(5); faces.verts.push_back(4);

	faces.verts.push_back(2); faces.verts.push_back(6); faces.verts.push_back(5);
	faces.verts.push_back(2); faces.verts.push_back(1); faces.verts.push_back(5);

	faces.verts.push_back(3); faces.verts.push_back(2); faces.verts.push_back(1);
	faces.verts.push_back(3); faces.verts.push_back(0); faces.verts.push_back(1);

	faces.verts.push_back(0); faces.verts.push_back(1); faces.verts.push_back(5);
	faces.verts.push_back(0); faces.verts.push_back(4); faces.verts.push_back(5);

	faces.verts.push_back(7); faces.verts.push_back(6); faces.verts.push_back(2);
	faces.verts.push_back(7); faces.verts.push_back(3); faces.verts.push_back(2);

	mesh = new Mesh(verts, norms, uvs, faces);
}

void Collider::genNormals() {
	switch (_type) {
	case ColliderType::SPHERE:
		break;
	case ColliderType::BOX:
		faceNormals.push_back(glm::vec3(1, 0, 0));//need to define some kind of vertex array for box colliders
		faceNormals.push_back(glm::vec3(0, 0, -1));
		faceNormals.push_back(glm::vec3(-1, 0, 0));
		faceNormals.push_back(glm::vec3(0, 0, 1));
		faceNormals.push_back(glm::vec3(0, 1, 0));
		faceNormals.push_back(glm::vec3(0, -1, 0));
		break;
	case ColliderType::MESH:
		//generate the face normals from the mesh's vertices
		std::vector<GLuint>& faceVerts = mesh->faces().verts;
		std::vector<glm::vec3> meshVerts = mesh->verts();
		int numFaces = faceVerts.size();
		for (int i = 0; i < numFaces; i += 3) {
			glm::vec3 normal, e1, e2, v;
			v = meshVerts[faceVerts[i]];
			e1 = meshVerts[faceVerts[i + 1]] - v;
			e2 = meshVerts[faceVerts[i + 2]] - v;
			normal = glm::normalize(glm::cross(e1, e2));
			faceNormals.push_back(normal);//so for each face (3 vertices), there is a normal in this vector. 
			//To get the first vertex in the face, multiply the index in this vector by 3 when using it with meshVerts
			//remember to first use it with faceVerts, then pass the result to meshVerts
		}
		for (int i = 0; i < numFaces; i += 3)
			addUniqueAxis(uniqueNormals, i / 3);
		break;
	}
	currNormals = std::vector<glm::vec3>(faceNormals.size());
}

void Collider::genEdges() {
	switch (_type) {
	case ColliderType::SPHERE:
		break;
	case ColliderType::BOX:
	case ColliderType::MESH:
		for (std::pair<std::string, std::vector<Adj>> pair : gauss.adjacencies) {
			for (int i = 0, numAdj = pair.second.size(); i < numAdj; i++) {
				setEdge(pair.second[i].edge, edges.size());
				edges.push_back(getVert(pair.second[i].edge[1]) - getVert(pair.second[i].edge[0]));
			}
		}
		break;
	}
	currEdges = std::vector<glm::vec3>(edges.size());
}

void Collider::genGaussMap() {
	switch (_type) {
	case ColliderType::SPHERE:
		break;
	case ColliderType::BOX:
		//need to set up proper handling for box colliders for vertices
		//gauss.addAdj(faceNormals[0], Adj{  });
		//gauss.adjacencies[faceNormals[1]] = { Adj{ 2, {,} } };
		break;
	case ColliderType::MESH:
		//set up the edge associations
		std::vector<GLuint>& faceVerts = mesh->faces().verts;
		int numFaces = faceVerts.size();
		for (int i = 0; i < numFaces; i += 3) {
			for (int j = i + 3; j < numFaces; j += 3) {
				
				if (fuzzyParallel(faceNormals[i / 3], faceNormals[j / 3]))
					continue;
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
								a.edge[1] = faceVerts[i + p1]; a.f1 = i / 3; a.f2 = j / 3;
								if (a.edge[0] > a.edge[1]) { a.edge[1] = a.edge[0]; a.edge[0] = faceVerts[i + p1]; }
								gauss.addAdj(faceNormals[a.f1], a);
								added = true;
							}
							break;//none of the other verts can be equal to this one now, so move to the next one
						}//end vert comparison

					}
				}//end edge loop

			}
		}//end face loop
		break;
	}
}

bool Collider::fuzzyParallel(glm::vec3 v1, glm::vec3 v2) {
	if (v1 == v2)
		return true;
	float propx = (v1.x != 0) ? v2.x / v1.x : 0;
	float propy = (v1.y != 0) ? v2.y / v1.y : 0;
	float eps = FLT_EPSILON;
	bool para = fabs((propy - propx) / propx) < eps && fabs((((v1.z != 0) ? v2.z / v1.z : 0) - propx) / propx) < eps;
	return para;
}

void Collider::addUniqueAxis(std::vector<int>& axes, int aIndex) {
	//this really should be optimized, but I'll worry about that later
	//std::vector<glm::vec3> meshNormals = mesh->normals();
	glm::vec3 axis = faceNormals[aIndex];
	//axis = glm::normalize(axis);//hmm
	int numAxes = axes.size();
	for (int i = 0; i < numAxes - 1; i++) {
		glm::vec3 v = faceNormals[axes[i]];
		if (fuzzyParallel(v,axis))
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
	switch (_type) {
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
	case ColliderType::MESH:
		Transform t = _transform->computeTransform();
		glm::mat4 rot = glm::rotate(t.rotAngle, t.rotAxis);
		//int numNormals = uniqueNormals.size();
		//auto faceVerts = mesh->faces().verts;
		for (int i = 0, numNormals = faceNormals.size(); i < numNormals; i++) {
			//currNormals[i] = (glm::vec3)(rot * glm::vec4(faceNormals[uniqueNormals[i]], 1));//this is probably slow
			currNormals[i] = (glm::vec3)(rot * glm::vec4(faceNormals[i], 1));
			
			/*glm::vec3 a = t.getTransformed(getVert(faceVerts[i * 3])), b = t.getTransformed(getVert(faceVerts[i * 3 + 1])), c = t.getTransformed(getVert(faceVerts[i * 3 + 2]));
			glm::vec3 center = a + b + c;
			center /= 3;
			DrawDebug::getInstance().drawDebugVector(center, center + currNormals[i]);*/
		}
		break;
	}
}

void Collider::updateEdges() {
	switch (_type) {
	case ColliderType::SPHERE:
		break;
	case ColliderType::BOX:
	case ColliderType::MESH:
		Transform t = _transform->computeTransform();
		glm::mat4 rot = glm::rotate(t.rotAngle, t.rotAxis);
		glm::mat4 scale = glm::scale(t.scale);
		int numEdges = edges.size();
		for (int i = 0; i < numEdges; i++) {
			currEdges[i] = (glm::vec3)(scale * rot * glm::vec4(edges[i], 1));//this is probably slow
		}
		for (std::pair<std::string, std::vector<Adj>> pair : gauss.adjacencies) {
			for (int i = 0, numAdj = pair.second.size(); i < numAdj; i++) {
				Adj a = pair.second[i];
				glm::vec3 s = t.getTransformed(getVert(a.edge[0]));
				glm::vec3 edge = getEdge(a.edge);
				DrawDebug::getInstance().drawDebugVector(s,s + edge);
			}
		}
		break;
	}
}

void Collider::update() {
	_framePos = _transform->computeTransform().position;
	updateNormals();
	updateEdges();
}

const std::vector<int>& Collider::getNormals() const { return uniqueNormals; }
const std::vector<glm::vec3>& Collider::getCurrNormals() const { return currNormals; }
const std::vector<glm::vec3>& Collider::getEdges() const { return currEdges; }

glm::vec3 Collider::getVert(int index) const { return mesh->verts()[index]; }
glm::vec3 Collider::getNormal(int index) const { return currNormals[index]; }
glm::vec3 Collider::getEdge(int (&e)[2]) { 
	if (e[1] < e[0]) { int temp = e[0]; e[0] = e[1]; e[1] = temp; }
	std::string key = std::to_string(e[0]) + "," + std::to_string(e[1]);
	return currEdges[edgeMap[key]];
}
void Collider::setEdge(int(&e)[2], int index) {
	if (e[1] < e[0]) { int temp = e[0]; e[0] = e[1]; e[1] = temp; }
	std::string key = std::to_string(e[0]) + "," + std::to_string(e[1]);
	edgeMap[key] = index;
}

const GaussMap& Collider::getGaussMap() const { return gauss; }

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

std::vector<glm::vec3> Collider::getAxes(const Collider* other) {
	std::vector<glm::vec3> combNormals;
	auto otherNormals = other->getCurrNormals();
	auto otheruniqueNormals = other->getNormals();
	for (int i = 0, numNormals = uniqueNormals.size(); i < numNormals; i++)
		combNormals.push_back(currNormals[uniqueNormals[i]]);
	for (int i = 0, numNormals = otheruniqueNormals.size(); i < numNormals; i++)
		combNormals.push_back(otherNormals[otheruniqueNormals[i]]);

	return combNormals;
}

std::vector<Adj>& GaussMap::getAdjs(glm::vec3 v) {
	std::string key = std::to_string(v.x) + "," + std::to_string(v.y) + "," + std::to_string(v.z);
	return adjacencies[key];
}

void GaussMap::addAdj(glm::vec3 v, Adj a) {
	std::string key = std::to_string(v.x) + "," + std::to_string(v.y) + "," + std::to_string(v.z);
	adjacencies[key].push_back(a);
}