#include "Mesh.h"
#include <iostream>

Mesh::Mesh(FaceData fd, FaceIndex fi) : _data(fd), _indices(fi) {}

inline float getDistSq(const vec3 v1, const vec3 v2) {
    const auto xDist = v1.x - v2.x;
    const auto yDist = v1.y - v2.y;
    const auto zDist = v1.z - v2.z;
    return xDist * xDist + yDist * yDist + zDist * zDist;
}

vec3 Mesh::getGrossDims() {
    return (h_dims.x > 0) ? h_dims : (h_dims = vec3(getGrossDim(_data.verts) * 0.5f));
}

vec3 Mesh::getPreciseDims() {
    return (h_dims.x > 0) ? h_dims : (h_dims = getPreciseDims(_data.verts) * 0.5f);
}

vec3 Mesh::getCentroid() {
    return getCentroid(_data.verts);
}

float Mesh::getGrossDim(const std::vector<vec3>& verts) {
    const auto centroid = getCentroid(verts);
    auto numVerts = verts.size();

    // find the most distant point from the center
    auto max = verts[0];
    auto maxDistSq = getDistSq(max, centroid);
    for (size_t i = 1; i < numVerts; ++i) {
        const auto v = verts[i];
        const auto distSq = getDistSq(v, centroid);
        if (distSq > maxDistSq) { max = v; maxDistSq = distSq; }
    }

    // find the most distant point from THAT point
    auto min = verts[0];
    maxDistSq = getDistSq(min, max);
    for (size_t i = 1; i < numVerts; ++i) {
        const auto v = verts[i];
        const auto distSq = getDistSq(v, max);
        if (distSq > maxDistSq) { min = v; maxDistSq = distSq; }
    }

    return glm::length(max - min);
}

vec3 Mesh::getPreciseDims(const std::vector<vec3>& verts) {
    auto numVerts = verts.size();

    // find the most distant coordinates from the center on all axes
    auto max = verts[0], absCache = glm::abs(max);
    for (size_t i = 1; i < numVerts; ++i) {
        const auto v = verts[i], vabs = glm::abs(v);
        if (vabs.x > absCache.x) { max.x = v.x; absCache.x = vabs.x; }
        if (vabs.y > absCache.y) { max.y = v.y; absCache.y = vabs.y; }
        if (vabs.z > absCache.z) { max.z = v.z; absCache.z = vabs.z; }
    }

    // find the most distant coordinates from THOSE coordinates
    auto min = verts[0];
    absCache = glm::abs(max - min);
    for (size_t i = 1; i < numVerts; ++i) {
        const auto v = verts[i], vabs = glm::abs(max - v);
        if (vabs.x > absCache.x) { min.x = v.x; absCache.x = vabs.x; }
        if (vabs.y > absCache.y) { min.y = v.y; absCache.y = vabs.y; }
        if (vabs.z > absCache.z) { min.z = v.z; absCache.z = vabs.z; }
    }

    return glm::abs(max - min);
}

vec3 Mesh::getCentroid(const std::vector<vec3>& verts) {
    vec3 centroid;
    for (const auto& vert : verts) centroid += vert;
    return centroid / (float)verts.size();
}

void Mesh::translate(const vec3 t) {
    const auto trans = glm::translate(t);
    for (auto& vert : _data.verts) vert = (vec3)(trans * vec4(vert, 1));
    resetRenderData();
}

void Mesh::translateTo(const vec3 t) {
    const auto c = getCentroid(), d = t - c;
    if (!(epsCheck(d.x) && epsCheck(d.y) && epsCheck(d.z))) translate(d);
}

void Mesh::scale(const vec3 s) {
    const auto sc = glm::scale(s);
    for (auto& vert : _data.verts) vert = (vec3)(sc * vec4(vert, 0));
    resetRenderData();
    if (s.x != s.y || s.x != s.z) {
        const auto inv_sc = inv_tp_tf(sc);
        for (auto& normal : _data.normals) normal = (vec3)(inv_sc * vec4(normal, 0));
    }
}

void Mesh::scaleTo(const vec3 s) {
    const auto old_s = getPreciseDims() * 2.f, d = s - old_s;
    if (!(epsCheck(d.x) && epsCheck(d.y) && epsCheck(d.z))) scale(s / old_s);
}

void Mesh::rotate(const quat& q) {
    const auto rot = glm::rotate(q.theta(), q.axis());
    for (auto& vert : _data.verts) vert = (vec3)(rot * vec4(vert, 0));
    for (auto& normal : _data.normals) normal = (vec3)(rot * vec4(normal, 0));
    resetRenderData();
}

shared<Mesh::RenderData> Mesh::getRenderData(bool needsTangents) {
    if (renderData)
        return renderData;
    
    std::vector<vec3> tangents;
    if (needsTangents) {
        tangents.reserve(_indices.verts.size() / 3);
        for (size_t i = 0, numVerts = _indices.verts.size(); i < numVerts; i += 3) {
            const auto v0 = _data.verts[_indices.verts[i]]
                     , v1 = _data.verts[_indices.verts[i + 1]]
                     , v2 = _data.verts[_indices.verts[i + 2]];
            const auto e1 = v1 - v0, e2 = v2 - v0;

            const vec2 u0 = _data.uvs[_indices.uvs[i]]
                     , u1 = _data.uvs[_indices.uvs[i + 1]]
                     , u2 = _data.uvs[_indices.uvs[i + 2]];
            const auto du1 = u1 - u0, du2 = u2 - u0;

            auto tangent = du2.y * e1 - du1.y * e2;
            tangent *= 1.f / (du1.x * du2.y + du2.x * du1.y);
            //printf("Combo: %s, Tangent: %s\n", to_string(vec3(_indices.verts[i], _indices.verts[i + 1], _indices.verts[i + 2])).c_str(), to_string(tangent).c_str());
            tangents.push_back(tangent);
        }
    }
    const auto floatsPerVert = needsTangents ? 11 : 8;

    const auto getCombIndex = [&](GLuint v, GLuint u, GLuint n) -> uint32_t {
        uint32_t index = 0;
        //TODO: fix this bottleneck! It's super duper slow!
        for (const auto f : _indices.combinations) {
            if (f.x == v && f.y == u && f.z == n) {
                break;
            }
            ++index;
        }
        return index;
    };

    RenderData render;
    for (size_t i = 0, numVerts = _indices.verts.size(); i < numVerts; ++i) {
        const auto v = _indices.verts[i], u = _indices.uvs[i], n = _indices.normals[i];
        auto index = getCombIndex(v, u, n);

        if (index == _indices.combinations.size()) {
            _indices.combinations.push_back({ v, u, n });
            render.vbuffer.reserve(render.vbuffer.size() + floatsPerVert);
            const auto vert = _data.verts[v], uv = _data.uvs[u], norm = _data.normals[n];
            render.vbuffer.push_back(vert.x); render.vbuffer.push_back(vert.y); render.vbuffer.push_back(vert.z);
            render.vbuffer.push_back(uv.x); render.vbuffer.push_back(uv.y);
            render.vbuffer.push_back(norm.x); render.vbuffer.push_back(norm.y); render.vbuffer.push_back(norm.z);
            if (needsTangents) {
                const auto tangent = tangents[v / 3];
                //printf("Vert: %s, Tangent: %s\n", to_string(vert).c_str(), to_string(tangent).c_str());
                render.vbuffer.push_back(tangent.x); render.vbuffer.push_back(tangent.y); render.vbuffer.push_back(tangent.z);
            }
        }
        render.ebuffer.push_back(index);
    }

    return renderData = make_shared<RenderData>(std::move(render));
}