#include "UV.h"

using namespace std;

void genUVs(Mesh::FaceData& data, Mesh::FaceIndex& indices) { genUVSpherical(data, indices); }

void genUVCylindrical(Mesh::FaceData& data, Mesh::FaceIndex& indices) {
	for (const auto index : indices.verts) {
		
		const auto vert = data.verts[index - 1];
		auto u = atan2f(vert.y, vert.x); // azimuth, atan2 does a lot of the work for you
		auto v = vert.z; // just z
		//u += 2 * PI; u -= (int)u;
		//v += 0.5f;
		auto uv = vec3(u, v, 0);

		//redundancy check
		auto uvIndex = find_index(data.uvs, uv);

		//if it's a new uv
		if (uvIndex >= data.uvs.size()) data.uvs.push_back(uv);

		indices.uvs.push_back(uvIndex + 1);
		//indices.uvs.push_back(uvIndex + 1);
		//indices.uvs.push_back(uvIndex + 1);
	}
}

void genUVSpherical(Mesh::FaceData& data, Mesh::FaceIndex& indices) {
	for (const auto index : indices.verts) {
		
		const auto vert = data.verts[index - 1];
		auto u = atan2f(vert.z, sqrt(std::pow(vert.x, 2) + std::pow(vert.y, 2))); // theta
		auto v = atan2f(vert.y, vert.x); // azimuth
		//u += 2 * PI; u -= (int) u;
		//v += 2 * PI; v -= (int) v;
		auto uv = vec3(u, v, 0);

		//redundancy check
		auto uvIndex = find_index(data.uvs, uv);

		//if it's a new uv
		if (uvIndex >= data.uvs.size()) data.uvs.push_back(uv);

		indices.uvs.push_back(uvIndex + 1);
		//indices.uvs.push_back(uvIndex + 1);
		//indices.uvs.push_back(uvIndex + 1);
	}
}