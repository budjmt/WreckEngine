#include "Mesh.h"
#include <iostream>

Mesh::Mesh(std::vector<vec3> v, std::vector<vec3> n, std::vector<vec3> u, Face f) : _verts(v), _normals(n), _uvs(u), _faces(f)
{	
	for (size_t i = 0, numVerts = _faces.verts.size(); i < numVerts; ++i) {
		//std::cout << i << std::endl;
		bool inArr = false;
		size_t index = 0;
		//TODO: fix this bottleneck! It's super duper slow!
		for (auto numCombs = _faces.combinations.size(); !inArr && index < numCombs; ++index) {
			if (_faces.combinations[index].x == _faces.verts[i] 
				&& _faces.combinations[index].y == _faces.uvs[i] 
				&& _faces.combinations[index].z == _faces.normals[i]) {
				inArr = true;
				--index;
			}
		}
		if (!inArr) {
			_faces.combinations.push_back(vec3(_faces.verts[i], _faces.uvs[i], _faces.normals[i]));
			meshArray.push_back(_verts[_faces.verts[i]].x);
			meshArray.push_back(_verts[_faces.verts[i]].y);
			meshArray.push_back(_verts[_faces.verts[i]].z);
			meshArray.push_back(_uvs[_faces.uvs[i]].x);
			meshArray.push_back(_uvs[_faces.uvs[i]].y);
			meshArray.push_back(_normals[_faces.normals[i]].x);
			meshArray.push_back(_normals[_faces.normals[i]].y);
			meshArray.push_back(_normals[_faces.normals[i]].z);
		}
		meshElementArray.push_back(index);
	}

	/*for (int i = 0; i < meshArray.size(); i += FLOATS_PER_VERT + FLOATS_PER_UV) {
		std::cout << i / (FLOATS_PER_VERT + FLOATS_PER_UV) << ": " << meshArray[i] << "," << meshArray[i + 1] << meshArray[i + 2] << " \\ "
			<< meshArray[i + FLOATS_PER_VERT] << "," << meshArray[i + 1 + FLOATS_PER_VERT] << std::endl;
	}
	std::cout << std::endl;

	int vertStride = FLOATS_PER_VERT;
	int totalStride = vertStride + FLOATS_PER_UV;
	for (int i = 0; i < meshElementArray.size(); i++) {
		std::cout << meshElementArray[i] << ": " << faces.combined[meshElementArray[i]].x << "/" << faces.combined[meshElementArray[i]].y << "/" << faces.combined[meshElementArray[i]].z << ": "
			<< meshArray[meshElementArray[i] * totalStride] << "," << meshArray[meshElementArray[i] * totalStride + 1] << "," << meshArray[meshElementArray[i] * totalStride + 2] << " \\ "
			<< meshArray[meshElementArray[i] * totalStride + vertStride] << "," << meshArray[meshElementArray[i] * totalStride + vertStride + 1] << std::endl;
		if ((i + 1) % 3 == 0)
			std::cout << std::endl;
	}*/
}

const std::vector<vec3>& Mesh::verts() const { return _verts; } void Mesh::verts(std::vector<vec3>& v) { _verts = v; }
const std::vector<vec3>& Mesh::uvs() const { return _uvs; } void Mesh::uvs(std::vector<vec3>& u) { _uvs = u; }
const std::vector<vec3>& Mesh::normals() const { return _normals; } void Mesh::normals(std::vector<vec3>& n) { _normals = n; }
Face Mesh::faces() const { return _faces; } void Mesh::faces(Face& f) { _faces = f; }

float getDistSq(vec3 v1, vec3 v2) {
	float xDist = v1.x - v2.x;
	float yDist = v1.y - v2.y;
	float zDist = v1.z - v2.z;
	return xDist * xDist + yDist * yDist + zDist * zDist;
}

vec3 Mesh::getDims() {
	if (h_dims.x > 0) return h_dims;

	// assumes the model is centered at 0,0,0
	auto center = vec3(0);

	// find the most distant point from the center
	auto max = _verts[0];
	auto maxDistSq = getDistSq(max, center);
	for (size_t i = 1, numVerts = _verts.size(); i < numVerts; ++i) {
		auto& v = _verts[i];
		auto distSq = getDistSq(v, center);
		if (distSq > maxDistSq) { max = v; maxDistSq = distSq; }
	}
	
	// find the most distant point from THAT point
	auto min = _verts[0];
	maxDistSq = getDistSq(min, max);
	for (size_t i = 1, numVerts = _verts.size(); i < numVerts; ++i) {
		auto& v = _verts[i];
		auto distSq = getDistSq(v, max);
		if (distSq > maxDistSq) { min = v; maxDistSq = distSq; }
	}

	h_dims = vec3(length(max - min) * 0.5f);
	return h_dims;
}