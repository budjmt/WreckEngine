#include "UV.h"

using namespace std;

void genUVs(std::vector<GLfloat>& verts, std::vector<GLuint>& vertFaces, std::vector<GLfloat>& uvs, std::vector<GLuint>& uvFaces) {
	genUVSpherical(verts, vertFaces, uvs, uvFaces);
}

void genUVCylindrical(std::vector<GLfloat>& verts, std::vector<GLuint>& vertFaces, std::vector<GLfloat>& uvs, std::vector<GLuint>& uvFaces) {
	for (size_t i = 0, numFaces = vertFaces.size(); i < numFaces; i++) {
		auto index = (vertFaces[i] - 1) * FLOATS_PER_VERT;
		auto u = atan2f(verts[index + 1], verts[index]);//azimuth, atan2 does a lot of the work for you
		auto v = verts[index + 2];//just z
		//u += 2 * PI; u -= (int)u;
		//v += 0.5f;
		auto uv = vec3(u, v, 0);

		//redundancy check
		int uvIndex = findIndexIn(uvs, FLOATS_PER_VERT, uv);

		if (uvIndex == uvs.size()) { //if it's a new uv
			uvs.push_back(uv[0]);
			uvs.push_back(uv[1]);
			uvs.push_back(uv[2]);
		}
		uvIndex /= FLOATS_PER_VERT;

		uvFaces.push_back(uvIndex + 1);
		//uvFaces.push_back(uvIndex + 1);
		//uvFaces.push_back(uvIndex + 1);
	}
}

void genUVSpherical(std::vector<GLfloat>& verts, std::vector<GLuint>& vertFaces, std::vector<GLfloat>& uvs, std::vector<GLuint>& uvFaces) {
	for (size_t i = 0, numFaces = vertFaces.size(); i < numFaces; i++) {
		auto index = (vertFaces[i] - 1) * FLOATS_PER_VERT;
		auto u = atan2f(verts[index + 2], sqrt(pow(verts[index], 2) + pow(verts[index + 1], 2)));//theta
		auto v = atan2f(verts[index + 1], verts[index]);//azimuth
		//u += 2 * PI; u -= (int) u;
		//v += 2 * PI; v -= (int) v;
		auto uv = vec3(u, v, 0);

		//redundancy check
		int uvIndex = findIndexIn(uvs, FLOATS_PER_VERT, uv);

		if (uvIndex == uvs.size()) { //if it's a new uv
			uvs.push_back(uv[0]);
			uvs.push_back(uv[1]);
			uvs.push_back(uv[2]);
		}
		uvIndex /= FLOATS_PER_NORM;

		uvFaces.push_back(uvIndex + 1);
		//uvFaces.push_back(uvIndex + 1);
		//uvFaces.push_back(uvIndex + 1);
	}
}