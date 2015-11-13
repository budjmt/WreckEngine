#pragma once

#include "GL/glew.h"
#include "glm/glm.hpp"

#include <vector>

#include "Drawable.h"

struct Face {
	//these all correspond to the indices in the vectors
	std::vector<GLuint> verts, uvs, normals;
	std::vector<glm::vec3> combined;
};

class Mesh
{
public:
	Mesh(std::vector<GLfloat> v, std::vector<GLfloat> n, std::vector<GLfloat> u, Face f);
	~Mesh(void);
	std::vector<GLfloat> verts() const; void verts(std::vector<GLfloat>& v);
	std::vector<GLfloat> uvs() const; void uvs(std::vector<GLfloat>& u);
	std::vector<GLfloat> normals() const; void normals(std::vector<GLfloat>& n);
	Face faces() const; void faces(Face& f);
protected:
	std::vector<GLfloat> mverts, mnormals, muvs;
	Face mfaces;
	std::vector<GLfloat> meshArray;
	std::vector<GLuint> meshElementArray;
	friend class DrawMesh;
};