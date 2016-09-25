#pragma once
#define _USE_MATH_DEFINES

#include "GL\glew.h"
#include "glm\glm.hpp"
#include "Mesh.h"
#include "UV.h"
#include <string>
#include <vector>

//char* loadFBX(const char* file);
shared<Mesh> loadOBJ(const char* file);//loads a .obj
void genOBJ(const char* file, std::vector<GLfloat>& verts, std::vector<GLfloat>& uvs, std::vector<GLfloat>& norms
	, std::vector<GLuint>& vertFaces, std::vector<GLuint>& uvFaces, std::vector<GLuint>& normFaces);

//general generation process is:
//generate vertices
//generate faces and normals
//generate uvs
void genCube(const char* file);
void genCylinder(const char* file, size_t res);
void genCone(const char* file, size_t res);
void genSphere(const char* file, size_t res);
//void genTorus(const char* file, float r1, float r2);//hole rad, ring rad
void genBezierSurface(const char* file, size_t ures, size_t vres, std::vector<std::vector<vec3>>& k);
vec3 bezierSurface(float u, float v, std::vector<std::vector<vec3>>& k);
float bernsteinPolynomial(int i, int n, float u);
float binomialCoeff(int n, int i);
int factorial(int n);

void genNormals(std::vector<GLfloat>& verts, std::vector<GLuint>& vertFaces, std::vector<GLfloat>& norms, std::vector<GLuint>& normFaces);
vec3 genNormal(vec3 v1, vec3 v2, vec3 v3);

size_t findIndexIn(std::vector<GLfloat>& vecs, size_t stride, vec3 vec);

std::vector<std::string> tokenize(std::string str, std::string delimiter);
