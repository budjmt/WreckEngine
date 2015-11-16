#pragma once

#include "GL/glew.h"
#include "glm/glm.hpp"

#include <vector>

#include "Drawable.h"

struct Face {
	//these all correspond to the indices in the vectors
	std::vector<GLuint> verts, uvs, normals;
	std::vector<glm::vec3> combinations;//all the unique v/u/n index combinations
};

class Mesh
{
public:
	Mesh(std::vector<glm::vec3> v, std::vector<glm::vec3> n, std::vector<glm::vec3> u, Face f);
	~Mesh(void);
	const std::vector<glm::vec3>& verts() const; void verts(std::vector<glm::vec3>& v);
	const std::vector<glm::vec3>& uvs() const; void uvs(std::vector<glm::vec3>& u);
	const std::vector<glm::vec3>& normals() const; void normals(std::vector<glm::vec3>& n);
	Face faces() const; void faces(Face& f);
protected:
	std::vector<glm::vec3> mverts, mnormals, muvs;
	Face mfaces;
	std::vector<GLfloat> meshArray;
	std::vector<GLuint> meshElementArray;
	friend class DrawMesh;
};