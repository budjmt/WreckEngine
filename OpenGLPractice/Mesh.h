#pragma once

#include "GL/glew.h"
#include "glm/glm.hpp"
#include "FreeImage.h"

#include <vector>

#include "Drawable.h"

struct Face {
	//these all correspond to the indices in the vectors
	std::vector<GLuint> verts, uvs, normals;
	std::vector<glm::vec3> combined;
};

class Mesh : public Drawable
{
public:
	Mesh(std::vector<GLfloat> v, std::vector<GLfloat> n, std::vector<GLfloat> u, Face f, char* texFile, GLuint shader);
	~Mesh(void);
	std::vector<GLfloat> verts, uvs, normals;
	Face faces;
	GLuint vertBuffer;
	void draw(GLfloat x, GLfloat y, GLfloat xScale, GLfloat yScale);
	void draw(glm::vec3 pos, glm::vec3 scale, glm::vec3 rotAxis, float rot);
private:
	//public right now because of weird initialization issue
	//std::vector<GLfloat> mverts, mnormals, mtCoords;
	//std::vector<Face> mfaces;
	GLuint textureLoc;
	std::vector<GLuint> textures;
	std::vector<GLfloat> meshArray;
	std::vector<GLuint> meshElementArray;
};