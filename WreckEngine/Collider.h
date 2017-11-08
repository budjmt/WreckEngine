#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "frame_cache.h"

#include "Transform.h"
#include "Mesh.h"

#include "DrawDebug.h"

class Collider;

struct AABB {
	vec3 center, halfDims;
	bool intersects(const AABB& other) const;
};

struct Edge {
    Edge() = default;
    Edge(const Edge&) = default;
    Edge(Edge&&) = default;
    Edge& operator=(const Edge&) = default;
    Edge& operator=(Edge&&) = default;

    Edge(GLuint a, GLuint b) noexcept : indices(a > b ? decltype(indices){ b, a } : decltype(indices){ a, b }) {}
    GLuint first()  const noexcept { return indices.first; }
    GLuint second() const noexcept { return indices.second; }

    bool operator==(const Edge& other) const noexcept { return indices == other.indices; }
private:
    std::pair<GLuint, GLuint> indices;
};
namespace std { template<> struct hash<Edge> { auto operator()(const Edge& t) const noexcept { return hash_bytes(t); } }; }

struct Adj { std::pair<GLuint, GLuint> faces; Edge edge; };

struct GaussMap {
	//keys are untransformed normals, adjacencies use indices because of rotations
	std::unordered_map<vec3, std::vector<Adj>> adjacencies;
	void addAdj(vec3 v, Adj a);
	const std::vector<Adj>& getAdjs(vec3 v) const;
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
    static constexpr float PEN_TOLERANCE = 0.03f * -1.f;
	enum class Type { SPHERE, BOX, MESH };

	Collider(Transform* t, const vec3 d, const bool fudge = true);
	Collider(shared<Mesh> m, Transform* t);
	
	const AABB& aabb = transformed_aabb;
	const Type& type = _type;
	void updateDims();

	Manifold intersects(Collider* other);

	std::vector<GLuint> getIncidentFaces(const vec3 refNormal);
	void clipPolygons(FaceManifold& reference, const std::vector<GLuint>& incidents);
	std::vector<vec3> clipPolyAgainstEdge(std::vector<vec3>& input, const vec3 sidePlane, const vec3 sideVert, const vec3 refNorm, const vec3 refCenter) const;
	vec3 closestPointBtwnSegments(const vec3 p0, const vec3 p1, const vec3 q0, const vec3 q1) const;

	void update();

	//void setCorners(std::vector<vec3> c);
	void genVerts();
	void genNormals();
	void genEdges();
	void genGaussMap();

    // these functions can update the frame-cache, and so aren't const
    const std::vector<vec3>& getCurrVerts();
	const std::vector<vec3>& getCurrNormals();
	const std::vector<vec3>& getCurrEdges();

	inline GLuint getFaceVert(const GLuint index) const;
	inline vec3 getVert(const GLuint index);
	inline vec3 getNormal(const GLuint index);
	inline vec3 getEdge(Edge e);

	const GaussMap& getGaussMap() const;

	bool fuzzyParallel(const vec3 v1, const vec3 v2) const;

	SupportPoint getSupportPoint(const vec3 dir);
	FaceManifold getAxisMinPen(Collider* other);
	EdgeManifold overlayGaussMaps(Collider* other);

private:
	Collider(const Type type, shared<Mesh> m, Transform* t, const vec3 d, const bool fudge = true);

    void updateVerts(std::vector<vec3>& currNormals);
    void updateNormals(std::vector<vec3>& currNormals);
    void updateEdges(std::vector<vec3>& currEdges);

	ACCS_G    (private, Transform*, transform);
	ACCS_G    (private, vec3,  framePos);
	ACCS_GS_C (private, vec3,  dims, { return _dims; }, { _dims = base_aabb.halfDims = value; updateDims(); });
	ACCS_G    (private, float, radius) = 0;

	bool fudgeAABB = true;//if this is true, the transformed AABB will be scaled by some factor
	AABB base_aabb, transformed_aabb;
	Type _type;

	std::vector<vec3> faceNormals, edges; // these are vec3s to avoid constant typecasting, and b/c cross product doesn't work for 4d vectors
    frame_cache<std::vector<vec3>, Collider> currVerts{ this, &Collider::updateVerts }, currNormals{ this, &Collider::updateNormals }, currEdges{ this, &Collider::updateEdges };
	std::unordered_map<Edge, GLuint> edgeMap; // maps the edge pairs to the indices in edges
	GaussMap gauss;
	shared<Mesh> mesh;

	void setEdge(Edge e, const GLuint index);
};