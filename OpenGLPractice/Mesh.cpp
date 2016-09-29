#include "Mesh.h"
#include <iostream>

Mesh::Mesh(std::vector<vec3> v, std::vector<vec3> n, std::vector<vec3> u, Face f) : _verts(v), _normals(n), _uvs(u), _faces(f)
{	
	for (size_t i = 0, numVerts = _faces.verts.size(); i < numVerts; ++i) {
		bool inArr = false;
		size_t index = 0;
		const auto v = _faces.verts[i], u = _faces.uvs[i], n = _faces.normals[i];
		//TODO: fix this bottleneck! It's super duper slow!
		for (const auto numCombs = _faces.combinations.size(); !inArr && index < numCombs; ++index) {
			const auto& f = _faces.combinations[index];
			if (f.x == v && f.y == u && f.z == n) {
				inArr = true;
				--index;
			}
		}
		if (!inArr) {
			const auto& vert = _verts[v], uv = _uvs[u], norm = _normals[n];
			_faces.combinations.push_back(vec3(v, u, n));
			meshArray.push_back(vert.x); meshArray.push_back(vert.y); meshArray.push_back(vert.z);
			meshArray.push_back(uv.x); meshArray.push_back(uv.y); 
			meshArray.push_back(norm.x); meshArray.push_back(norm.y); meshArray.push_back(norm.z);
		}
		meshElementArray.push_back(index);
	}
}

inline float getDistSq(const vec3 v1, const vec3 v2) {
	const auto xDist = v1.x - v2.x;
	const auto yDist = v1.y - v2.y;
	const auto zDist = v1.z - v2.z;
	return xDist * xDist + yDist * yDist + zDist * zDist;
}

vec3 Mesh::getGrossDims() {
	if (h_dims.x > 0) return h_dims;

	const auto centroid = getCentroid();

	// find the most distant point from the center
	auto max = _verts[0];
	auto maxDistSq = getDistSq(max, centroid);
	for (size_t i = 1, numVerts = _verts.size(); i < numVerts; ++i) {
		const auto v = _verts[i];
		const auto distSq = getDistSq(v, centroid);
		if (distSq > maxDistSq) { max = v; maxDistSq = distSq; }
	}

	// find the most distant point from THAT point
	auto min = _verts[0];
	maxDistSq = getDistSq(min, max);
	for (size_t i = 1, numVerts = _verts.size(); i < numVerts; ++i) {
		const auto v = _verts[i];
		const auto distSq = getDistSq(v, max);
		if (distSq > maxDistSq) { min = v; maxDistSq = distSq; }
	}

	h_dims = vec3(glm::length(max - min) * 0.5f);
	return h_dims;
}

vec3 Mesh::getPreciseDims() {
	if (h_dims.x > 0) return h_dims;

	// find the most distant coordinates from the center on all axes
	auto max = _verts[0], absCache = glm::abs(max);
	for (size_t i = 1, numVerts = _verts.size(); i < numVerts; ++i) {
		const auto v = _verts[i], vabs = glm::abs(v);
		if (vabs.x > absCache.x) { max.x = v.x; absCache.x = vabs.x; }
		if (vabs.y > absCache.y) { max.y = v.y; absCache.y = vabs.y; }
		if (vabs.z > absCache.z) { max.z = v.z; absCache.z = vabs.z; }
	}
	
	// find the most distant coordinates from THOSE coordinates
	auto min = _verts[0];
	absCache = glm::abs(max - min);
	for (size_t i = 1, numVerts = _verts.size(); i < numVerts; ++i) {
		const auto v = _verts[i], vabs = glm::abs(max - v);
		if (vabs.x > absCache.x) { min.x = v.x; absCache.x = vabs.x; }
		if (vabs.y > absCache.y) { min.y = v.y; absCache.y = vabs.y; }
		if (vabs.z > absCache.z) { min.z = v.z; absCache.z = vabs.z; }
	}

	h_dims = glm::abs(max - min) * 0.5f;
	return h_dims;
}

vec3 Mesh::getCentroid() {
	vec3 centroid;
	for (const auto& vert : _verts) centroid += vert;
	return centroid / (float)_verts.size();
}

void Mesh::translate(const vec3 t) {
	const auto trans = glm::translate(t);
	for (auto& vert : _verts) vert = (vec3)(trans * vec4(vert, 1));
}

void Mesh::translateTo(const vec3 t) {
	const auto c = getCentroid(), d = t - c;
	if(!(EPS_CHECK(d.x) && EPS_CHECK(d.y) && EPS_CHECK(d.z))) translate(d);
}

void Mesh::scale(const vec3 s) {
	const auto sc = glm::scale(s);
	for (auto& vert : _verts) vert = (vec3)(sc * vec4(vert, 0));
	if (s.x != s.y || s.x != s.z) {
		const auto inv_sc = inv_tp_tf(sc);
		for (auto& normal : _normals) normal = (vec3)(inv_sc * vec4(normal, 0));
	}
}

void Mesh::scaleTo(const vec3 s) {
	const auto old_s = getPreciseDims() * 2.f, d = s - old_s;
	if (!(EPS_CHECK(d.x) && EPS_CHECK(d.y) && EPS_CHECK(d.z))) scale(s / old_s);
}

void Mesh::rotate(const quat q) {
	const auto rot = glm::rotate(q.theta(), q.axis());
	for (auto& vert : _verts) vert = (vec3)(rot * vec4(vert, 0));
	for (auto& normal : _normals) normal = (vec3)(rot * vec4(normal, 0));
}