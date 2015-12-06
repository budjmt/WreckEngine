#include "Mesh.h"
#include <iostream>

Mesh::Mesh(std::vector<glm::vec3> v, std::vector<glm::vec3> n, std::vector<glm::vec3> u, Face f)
{
	verts(v);
	normals(n);
	uvs(u);
	faces(f);

	int nu_verts = verts().size();
	int nu_uvs = uvs().size();
	
	for (unsigned int i = 0; i < _faces.verts.size(); i++) {
		//std::cout << i << std::endl;
		bool inArr = false;
		unsigned int index;
		for (index = 0; !inArr && index < _faces.combinations.size();index++) {
			if (_faces.combinations[index].x == _faces.verts[i] 
				&& _faces.combinations[index].y == _faces.uvs[i] 
				&& _faces.combinations[index].z == _faces.normals[i]) {
				inArr = true;
				index--;
			}
		}
		if (!inArr) {
			_faces.combinations.push_back(glm::vec3(_faces.verts[i], _faces.uvs[i], _faces.normals[i]));
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

	h_dims = glm::vec3(-1, -1, -1);
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

Mesh::~Mesh()
{
	//delete[] meshArray;
	//delete[] meshElementArray;
}

const std::vector<glm::vec3>& Mesh::verts() const { return _verts; } void Mesh::verts(std::vector<glm::vec3>& v) { _verts = v; }
const std::vector<glm::vec3>& Mesh::uvs() const { return _uvs; } void Mesh::uvs(std::vector<glm::vec3>& u) { _uvs = u; }
const std::vector<glm::vec3>& Mesh::normals() const { return _normals; } void Mesh::normals(std::vector<glm::vec3>& n) { _normals = n; }
Face Mesh::faces() const { return _faces; } void Mesh::faces(Face& f) { _faces = f; }

float getDistSq(glm::vec3 v1, glm::vec3 v2) {
	float xDist = v1.x - v2.x;
	float yDist = v1.y - v2.y;
	float zDist = v1.z - v2.z;
	return xDist * xDist + yDist * yDist + zDist * zDist;
}

glm::vec3 Mesh::getDims() {
	//right now this assumes the model is centered at 0,0,0
	if (h_dims.x > 0)
		return h_dims;
	glm::vec3 center = glm::vec3(0, 0, 0);
	//find the most distant point from the center
	glm::vec3 max = _verts[0];
	float maxDistSq = getDistSq(max, center);
	for (int i = 1, numVerts = _verts.size(); i < numVerts; i++) {
		glm::vec3 v = _verts[i];
		float distSq = getDistSq(v, center);
		if (distSq > maxDistSq) { max = v; maxDistSq = distSq; }
	}
	//this finds the most distant point from THAT most distant point
	glm::vec3 min = _verts[0];
	maxDistSq = getDistSq(min, max);
	for (int i = 1, numVerts = _verts.size(); i < numVerts; i++) {
		glm::vec3 v = _verts[i];
		float distSq = getDistSq(v, max);
		if (distSq > maxDistSq) {
			min = v;
			maxDistSq = distSq;
		}
	}

	float radius = glm::length(max - min) * 0.5f;
	glm::vec3 d = glm::vec3(radius, radius, radius);
	h_dims = d;
	return h_dims;
}