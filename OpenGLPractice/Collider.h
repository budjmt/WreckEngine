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
	void addAdj(const vec3 v, const Adj a);
	const std::vector<Adj>& getAdjs(const vec3 v) const;
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
	Collider(Transform* t, const vec3 d, const bool fudge = true);
	Collider(Mesh* m, Transform* t);
	
	const AABB& aabb = transformed_aabb;
	const ColliderType& type = _type;
	void updateDims();

	Manifold intersects(Collider* other);

	std::vector<GLuint> getIncidentFaces(const vec3 refNormal) const;
	void clipPolygons(FaceManifold& reference, const std::vector<GLuint>& incidents) const;
	std::vector<vec3> clipPolyAgainstEdge(std::vector<vec3>& input, const vec3 sidePlane, const vec3 sideVert, const vec3 refNorm, const vec3 refCenter) const;
	vec3 closestPointBtwnSegments(const vec3 p0, const vec3 p1, const vec3 q0, const vec3 q1) const;

	void update();

	//void setCorners(std::vector<vec3> c);
	void genVerts();//only used for box colliders right now
	void genNormals();
	void genEdges();
	void genGaussMap();

	const std::vector<vec3>& getCurrNormals() const;
	const std::vector<vec3>& getEdges() const;

	inline GLuint getFaceVert(const GLuint index) const;
	inline vec3 getVert(const GLuint index) const;
	inline vec3 getNormal(const GLuint index) const;
	inline vec3 getEdge(std::pair<GLuint, GLuint> e) const;

	const GaussMap& getGaussMap() const;

	void updateNormals();
	void updateEdges();

	bool fuzzyParallel(const vec3 v1, const vec3 v2) const;

	SupportPoint getSupportPoint(const vec3 dir) const;
	FaceManifold getAxisMinPen(Collider* other);
	EdgeManifold overlayGaussMaps(Collider* other);

private:
	Collider(const ColliderType type, Mesh* m, Transform* t, const vec3 d, const bool fudge = true);

	ACCS_G    (private, Transform*, transform);
	ACCS_G    (private, vec3,  framePos);
	ACCS_GS_C (private, vec3,  dims, { return _dims; }, { _dims = base_aabb.halfDims = value; updateDims(); });
	ACCS_G    (private, float, radius) = 0;

	bool fudgeAABB = true;//if this is true, the transformed AABB will be scaled by some factor
	AABB base_aabb, transformed_aabb;
	ColliderType _type;

	std::vector<vec3> faceNormals, currNormals, edges, currEdges;//these are vec3s to avoid constant typecasting, and b/c cross product doesn't work for 4d vectors
	std::unordered_map<std::string, GLuint> edgeMap;//maps the edge pairs to the indices in edges
	GaussMap gauss;
	Mesh* mesh;

	void setEdge(std::pair<GLuint, GLuint> e, const GLuint index);
};