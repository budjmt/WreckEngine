#include "Mesh.h"
#include <iostream>

Mesh::Mesh(std::vector<GLfloat> v, std::vector<GLfloat> n, std::vector<GLfloat> u, Face f)
{
	verts(v);
	normals(n);
	uvs(u);
	faces(f);

	int numVerts = verts().size() / FLOATS_PER_VERT;
	int numUvs = uvs().size() / FLOATS_PER_UV;
	
	for (unsigned int i = 0; i < mfaces.verts.size(); i++) {
		//std::cout << i << std::endl;
		bool inArr = false;
		unsigned int index;
		for (index = 0; !inArr && index < mfaces.combined.size();index++) {
			if (mfaces.combined[index].x == mfaces.verts[i] && mfaces.combined[index].y == mfaces.uvs[i] && mfaces.combined[index].z == mfaces.normals[i]) {
				inArr = true;
				index--;
			}
		}
		if (!inArr) {
			mfaces.combined.push_back(glm::vec3(mfaces.verts[i], mfaces.uvs[i], mfaces.normals[i]));
			meshArray.push_back(mverts[mfaces.verts[i] * FLOATS_PER_VERT]);
			meshArray.push_back(mverts[mfaces.verts[i] * FLOATS_PER_VERT + 1]);
			meshArray.push_back(mverts[mfaces.verts[i] * FLOATS_PER_VERT + 2]);
			meshArray.push_back(muvs[mfaces.uvs[i] * FLOATS_PER_UV]);
			meshArray.push_back(muvs[mfaces.uvs[i] * FLOATS_PER_UV + 1]);
			meshArray.push_back(mnormals[mfaces.normals[i] * FLOATS_PER_NORM]);
			meshArray.push_back(mnormals[mfaces.normals[i] * FLOATS_PER_NORM + 1]);
			meshArray.push_back(mnormals[mfaces.normals[i] * FLOATS_PER_NORM + 2]);
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

Mesh::~Mesh()
{
	//delete[] meshArray;
	//delete[] meshElementArray;
}

std::vector<GLfloat> Mesh::verts() const { return mverts; } void Mesh::verts(std::vector<GLfloat>& v) { mverts = v; }
std::vector<GLfloat> Mesh::uvs() const { return muvs; } void Mesh::uvs(std::vector<GLfloat>& u) { muvs = u; }
std::vector<GLfloat> Mesh::normals() const { return mnormals; } void Mesh::normals(std::vector<GLfloat>& n) { mnormals = n; }
Face Mesh::faces() const { return mfaces; } void Mesh::faces(Face& f) { mfaces = f; }