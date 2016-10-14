#pragma once

#include "Mesh.h"
#include "ModelHelper.h"

void genUVs(Mesh::FaceData& data, Mesh::FaceIndex& indices);
void genUVCylindrical(Mesh::FaceData& data, Mesh::FaceIndex& indices);
void genUVSpherical(Mesh::FaceData& data, Mesh::FaceIndex& indices);
//void genUVCubic(Mesh::FaceData& data, Mesh::FaceIndex& indices);