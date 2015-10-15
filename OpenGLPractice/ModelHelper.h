#pragma once
#include "GL\glew.h"
#include "glm\glm.hpp"
#include "Mesh.h"
#include <string>
#include <vector>

//char* loadFBX(const char* file);
Mesh* loadOBJ(const char* file, char* texture, GLuint shader);//loads a .obj
void genOBJ(const char* file, std::vector<GLfloat> v, std::vector<GLfloat> uv, std::vector<GLfloat> n, std::vector<GLuint> f);

//general generation process is:
//generate vertices
//generate faces and normals
//generate uvs
void genCube(const char* file, float w, float h, float d);
void genCylinder(const char* file, float r, float h);
//void genCone(const car* file, float r, float h);
void genSphere(const char* file, float r);
//void genTorus(const char* file, float r1, float r2);//hole rad, ring rad
void genBezierSurface();

std::vector<GLfloat> genUVs(std::vector<GLfloat>& verts);
std::vector<GLfloat> genCylindrical(std::vector<GLfloat>& verts);
std::vector<GLfloat> genSpherical(std::vector<GLfloat>& verts);
//std::vector<GLfloat> genCubic(std::vector<GLfloat>& verts, std::vector<GLfloat>& norms);
std::vector<GLfloat> genNormals(std::vector<GLfloat>& verts, std::vector<GLuint>& vertFaces
								, std::vector<GLfloat>& norms, std::vector<GLuint>& normFaces);
GLfloat* genNormal(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3);

std::vector<std::string> tokenize(std::string str, std::string delimiter);
