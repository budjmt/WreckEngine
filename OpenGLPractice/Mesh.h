#pragma once

#include "GL/glew.h"

#include <vector>

#include "MarchMath.h"

#include "Drawable.h"

struct Face {
	//these all correspond to the indices in the vectors
	std::vector<GLuint> verts, uvs, normals;
	std::vector<vec3> combinations;//all the unique v/u/n index combinations
};

class Mesh
{
public:
	Mesh(std::vector<vec3> v, std::vector<vec3> n, std::vector<vec3> u, Face f);
	const std::vector<vec3>& verts()   const; void verts(std::vector<vec3>& v);
	const std::vector<vec3>& uvs()     const; void uvs(std::vector<vec3>& u);
	const std::vector<vec3>& normals() const; void normals(std::vector<vec3>& n);
	Face faces() const; void faces(Face& f);
	vec3 getDims();
protected:
	std::vector<vec3> _verts, _normals, _uvs;
	Face _faces;

	std::vector<GLfloat> meshArray;
	std::vector<GLuint> meshElementArray;
	
	vec3 h_dims = vec3(-1);
	friend class DrawMesh;
};