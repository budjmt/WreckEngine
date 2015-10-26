#pragma once
#define _USE_MATH_DEFINES

#include "GL\glew.h"
#include "glm\glm.hpp"
#include "Mesh.h"
#include "UV.h"
#include <string>
#include <vector>

//char* loadFBX(const char* file);
Mesh* loadOBJ(const char* file, char* texture, GLuint shader);//loads a .obj
void genOBJ(const char* file, std::vector<GLfloat> v, std::vector<GLfloat> uv, std::vector<GLfloat> n, std::vector<GLuint> f);

//general generation process is:
//generate vertices
//generate faces and normals
//generate uvs
void genCube(const char* file);
void genCylinder(const char* file, int res);
//void genCone(const car* file, int res);
void genSphere(const char* file, int res);
//void genTorus(const char* file, float r1, float r2);//hole rad, ring rad
void genBezierSurface(const char* file, int ures, int vres, std::vector<std::vector<glm::vec3>>& k);
glm::vec3 bezierSurface(float u, float v, std::vector<std::vector<glm::vec3>>& k);
float bernsteinPolynomial(int i, int n, float u);
float binomialCoeff(int n, int i);
int factorial(int n);

void genNormals(std::vector<GLfloat>& verts, std::vector<GLuint>& vertFaces
					, std::vector<GLfloat>& norms, std::vector<GLuint>& normFaces);
glm::vec3 genNormal(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3);

int findIndexIn(std::vector<GLfloat>& vecs, int stride, glm::vec3 vec);

std::vector<std::string> tokenize(std::string str, std::string delimiter);
