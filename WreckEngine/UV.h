#pragma once

#include "Mesh.h"
#include "ModelHelper.h"

void genUVs(std::vector<GLfloat>& verts, std::vector<GLuint>& vertFaces, std::vector<GLfloat>& uvs, std::vector<GLuint>& uvFaces);
void genUVCylindrical(std::vector<GLfloat>& verts, std::vector<GLuint>& vertFaces, std::vector<GLfloat>& uvs, std::vector<GLuint>& uvFaces);
void genUVSpherical(std::vector<GLfloat>& verts, std::vector<GLuint>& vertFaces, std::vector<GLfloat>& uvs, std::vector<GLuint>& uvFaces);
//void genUVCubic(std::vector<GLfloat>& verts, std::vector<GLuint>& vertFaces, std::vector<GLfloat>& norms, std::vector<GLfloat>& uvs, std::vector<GLuint>& uvFaces);