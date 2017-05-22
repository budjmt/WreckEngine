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
    renderData.reset();
}

void Mesh::translateTo(const vec3 t) {
    const auto c = getCentroid(), d = t - c;
    if (!(epsCheck(d.x) && epsCheck(d.y) && epsCheck(d.z))) translate(d);
}

void Mesh::scale(const vec3 s) {
    const auto sc = glm::scale(s);
    for (auto& vert : _data.verts) vert = (vec3)(sc * vec4(vert, 0));
    renderData.reset();
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
    renderData.reset();
}

shared<Mesh::RenderData> Mesh::getRenderData() {
    if (renderData)
        return renderData;

    RenderData render;
    for (size_t i = 0, numVerts = _indices.verts.size(); i < numVerts; ++i) {
        bool inArr = false;
        size_t index = 0;
        const auto v = _indices.verts[i], u = _indices.uvs[i], n = _indices.normals[i];
        //TODO: fix this bottleneck! It's super duper slow!
        for (const auto numCombs = _indices.combinations.size(); !inArr && index < numCombs; ++index) {
            const auto f = _indices.combinations[index];
            if (f.x == v && f.y == u && f.z == n) {
                inArr = true;
                --index;
            }
        }
        if (!inArr) {
            const auto vert = _data.verts[v], uv = _data.uvs[u], norm = _data.normals[n];
            _indices.combinations.push_back(vec3(v, u, n));
            render.vbuffer.push_back(vert.x); render.vbuffer.push_back(vert.y); render.vbuffer.push_back(vert.z);
            render.vbuffer.push_back(uv.x); render.vbuffer.push_back(uv.y);
            render.vbuffer.push_back(norm.x); render.vbuffer.push_back(norm.y); render.vbuffer.push_back(norm.z);
        }
        render.ebuffer.push_back(index);
    }

    return renderData = make_shared<RenderData>(render);
}