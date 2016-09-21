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
	vec3 getDims();
protected:
	ACCS_GS_T (protected, std::vector<vec3>, const std::vector<vec3>&, const std::vector<vec3>&, verts);
	ACCS_GS_T (protected, std::vector<vec3>, const std::vector<vec3>&, const std::vector<vec3>&, normals);
	ACCS_GS_T (protected, std::vector<vec3>, const std::vector<vec3>&, const std::vector<vec3>&, uvs);
	ACCS_GS_T (protected, Face, const Face&, const Face&, faces);

	std::vector<GLfloat> meshArray;
	std::vector<GLuint> meshElementArray;
	
	vec3 h_dims = vec3(-1);
	friend class DrawMesh;
};