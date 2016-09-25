#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "Transform.h"
#include "Mesh.h"

#include "DrawDebug.h"

class Collider;

enum class ColliderType { SPHERE, BOX, MESH };

const float COLLISION_PEN_TOLERANCE = 0.01f * -1.f;

struct AABB {
	vec3 center, halfDims;
	bool intersects(const AABB& other) const;
};

struct Adj { std::pair<GLuint, GLuint> faces, edge; };

struct GaussMap {
	//keys are untransformed normals, adjs use indices because of rotations
	std::unordered_map<std::string, std::vector<Adj>> adjacencies;
	void addAdj(vec3 v, Adj a);
	std::vector<Adj>& getAdjs(vec3 v);
};

struct SupportPoint { vec3 point; float proj; };

struct Manifold {
	Collider* originator = nullptr, *other = nullptr;
	std::vector<vec3> colPoints;
	float pen = -FLT_MAX;
	vec3 axis;
};

struct FaceManifold : Manifold { GLuint norm; };
struct EdgeManifold : Manifold { std::pair<Adj, Adj> edgePair; };

class Collider
{
public:
	Collider(Transform* t, vec3 d, bool fudge = true);
	Collider(Mesh* m, Transform* t);
	
	AABB& aabb = transformed_aabb;
	ColliderType& type = _type;
	void updateDims();

	Manifold intersects(Collider* other);

	std::vector<GLuint> getIncidentFaces(vec3 refNormal);
	void clipPolygons(FaceManifold& reference, std::vector<GLuint>& incidents);
	std::vector<vec3> clipPolyAgainstEdge(std::vector<vec3>& input, vec3 sidePlane, vec3 sideVert, vec3 refNorm, vec3 refCenter);
	vec3 closestPointBtwnSegments(vec3 p0, vec3 p1, vec3 q0, vec3 q1);

	void update();

	//void setCorners(std::vector<vec3> c);
	void genVerts();//only used for box colliders right now
	void genNormals();
	void genEdges();
	void genGaussMap();

	const std::vector<vec3>& getCurrNormals() const;
	const std::vector<vec3>& getEdges() const;

	inline int getFaceVert(size_t index) const;
	inline vec3 getVert(size_t index) const;
	inline vec3 getNormal(size_t index) const;
	inline vec3 getEdge(std::pair<GLuint, GLuint> e);

	const GaussMap& getGaussMap() const;

	void updateNormals();
	void updateEdges();

	bool fuzzyParallel(vec3 v1, vec3 v2);

	SupportPoint getSupportPoint(vec3 dir);
	FaceManifold getAxisMinPen(Collider* other);
	EdgeManifold overlayGaussMaps(Collider* other);

private:
	Collider(ColliderType type, Mesh* m, Transform* t, vec3 d, bool fudge = true);

	ACCS_G    (private, Transform*, transform);
	ACCS_G    (private, vec3,  framePos);
	ACCS_GS_C (private, vec3,  dims, { return _dims; }, { _dims = base_aabb.halfDims = value; updateDims(); });
	ACCS_G    (private, float, radius) = 0;

	alloc<aligned_mat4> normalMatCache = alloc<aligned_mat4>(new aligned_mat4(0.f));
	vec3 oldScale;

	bool fudgeAABB = true;//if this is true, the transformed aabb will be scaled by some factor
	AABB base_aabb, transformed_aabb;
	ColliderType _type;

	std::vector<vec3> faceNormals, currNormals, edges, currEdges;//these are vec3s to avoid constant typecasting, and b/c cross product doesn't work for 4d vectors
	std::unordered_map<std::string, GLuint> edgeMap;//maps the edge pairs to the indices in edges
	GaussMap gauss;
	//std::vector<std::vector<Adj>> adjacencies;
	Mesh* mesh;

	void setEdge(std::pair<GLuint, GLuint> e, size_t index);
};