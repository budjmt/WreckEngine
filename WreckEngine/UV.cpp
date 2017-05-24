#include "UV.h"

// https://stackoverflow.com/questions/34958072/programatically-generate-simple-uv-mapping-for-models

void genUVs(Mesh::FaceData& data, Mesh::FaceIndex& indices) { genUVSpherical(data, indices); }

void genUVCylindrical(Mesh::FaceData& data, Mesh::FaceIndex& indices) {
    for (const auto index : indices.verts) {

        const auto vert = data.verts[index - 1];
        auto u = atan2f(vert.x, vert.z) / PI * 0.5f + 0.5f; // azimuth, atan2 does a lot of the work for you
        auto v = vert.y; // just z
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

        const auto vert = glm::normalize(data.verts[index - 1]);
        auto u = atan2f(vert.z, vert.x) * 0.5f; // theta
        auto v = asin(vert.y); // azimuth
        auto uv = vec3(vec2(u, v) * (1.f / PI) - 0.5f, 0);
        uv.y *= -1.f;

        //redundancy check
        auto uvIndex = find_index(data.uvs, uv);

        //if it's a new uv
        if (uvIndex >= data.uvs.size()) data.uvs.push_back(uv);

        indices.uvs.push_back(uvIndex + 1);
        //indices.uvs.push_back(uvIndex + 1);
        //indices.uvs.push_back(uvIndex + 1);
    }
}