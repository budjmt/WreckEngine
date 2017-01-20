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
void genOBJ(const char* file, Mesh::FaceData& data, Mesh::FaceIndex& indices);

//general generation process is:
//generate vertices
//generate faces and normals
//generate UVs
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

void genNormals(Mesh::FaceData& data, Mesh::FaceIndex& indices);
vec3 genNormal(const vec3 a, const vec3 b, const vec3 c);

template<typename T> size_t find_index(const std::vector<T>& container, const T item) { return std::find(container.begin(), container.end(), item) - container.begin(); }