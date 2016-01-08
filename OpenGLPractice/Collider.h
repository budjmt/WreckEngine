#pragma once

#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/constants.hpp"

#include<string>
#include <vector>

#include "Transform.h"
#include "Mesh.h"

#include "DrawDebug.h"

class Collider;

enum class ColliderType {
	//CIRCLE,
	//RECT,
	//TRIANGLE
	SPHERE,
	BOX,
	MESH
};

struct Adj {
	int f1, f2;
	int edge[2];
};

struct GaussMap {
	//std::vector<glm::vec3> normals;
	std::map<std::string, std::vector<Adj>> adjacencies;//keys are untransformed normals, adjs use indices because of rotations
	void addAdj(glm::vec3 v, Adj a);
	std::vector<Adj>& getAdjs(glm::vec3 v);
};

struct SupportPoint {
	glm::vec3 point;
	float proj;
};

struct Manifold {
	Collider* originator, * other;
	std::vector<glm::vec3> colPoints;
	float pen = -FLT_MAX;
};

struct FaceManifold : Manifold {
	int axis;
};

struct EdgeManifold : Manifold {
	Adj edgePair[2];
};

class Collider
{
public:
	Collider(void);
	Collider(Transform* t, glm::vec3 b);
	Collider(Mesh* m, Transform* t);
	Collider(const Collider& other);
	~Collider(void);
	ColliderType& type;
	Transform* transform();
	glm::vec3 framePos();
	glm::vec3 dims(); void dims(glm::vec3 v);
	float radius() const;

	bool intersects2D(Collider* other);
	Manifold intersects(Collider* other);

	std::vector<int> getIncidentFaces(glm::vec3 refNormal);
	void clipPolygons(FaceManifold& reference, std::vector<int>& incidents);
	std::vector<glm::vec3> clipPolyAgainstEdge(std::vector<glm::vec3>& input, glm::vec3 sidePlane, glm::vec3 sideVert, glm::vec3 refNorm, glm::vec3 refCenter);
	glm::vec3 closestPointBtwnSegments(glm::vec3 p0, glm::vec3 p1, glm::vec3 q0, glm::vec3 q1);

	void update();

	//void setCorners(std::vector<glm::vec3> c);
	void genVerts();//only used for box colliders right now
	void genNormals();
	void genEdges();
	void genGaussMap();
	void addUniqueAxis(std::vector<int>& axes, int aIndex);
	
	const std::vector<int>& getNormals() const;
	const std::vector<glm::vec3>& getCurrNormals() const;
	const std::vector<glm::vec3>& getEdges() const;
	
	int getFaceVert(int index) const;
	glm::vec3 getVert(int index) const;
	glm::vec3 getNormal(int index) const;
	glm::vec3 getEdge(int (&e)[2]);
	
	const GaussMap& getGaussMap() const;
	
	void updateNormals();
	void updateEdges();

	void getMaxMin(glm::vec3 axis,float* maxmin);

	bool fuzzyParallel(glm::vec3 v1, glm::vec3 v2);

	SupportPoint getSupportPoint(glm::vec3 dir);
	FaceManifold getAxisMinPen(Collider* other);
	EdgeManifold overlayGaussMaps(Collider* other);
	
	std::vector<glm::vec3> getAxes(const Collider* other);
private:
	Transform* _transform;
	glm::vec3 _framePos;
	glm::vec3 _dims;
	float _radius;
	ColliderType _type;

	std::vector<glm::vec3> faceNormals, currNormals, edges, currEdges;//these are vec3s to avoid constant typecasting, and b/c cross product doesn't work for 4d vectors
	std::vector<int> uniqueNormals;//indices of the faceNormals that are unique
	std::map<std::string, int> edgeMap;//maps the edge pairs to the indices in edges
	GaussMap gauss;
	//std::vector<std::vector<Adj>> adjacencies;
	Mesh* mesh;

	void setEdge(int(&e)[2], int index);
};