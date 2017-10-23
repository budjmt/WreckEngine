#pragma once

#include "GL/glew.h"
#include "glm/glm.hpp"

#include <vector>

#include "gl_structs.h"

#include "Renderable.h"
#include "Camera.h"
#include "ShaderHelper.h"
#include "ModelHelper.h"
#include "Mesh.h"

#include <functional>
#include "safe_queue.h"

//#if defined(_DEBUG) && !defined(NDEBUG)
#define DEBUG true
//#else
//#define DEBUG false
//#endif

constexpr size_t MAX_VECTORS = 2500;
constexpr size_t MAX_SPHERES = 1000;
constexpr size_t MAX_BOXES = 1000;

template<class T>
struct InstMesh { 
    std::vector<T> instances;

    InstMesh() = default;
    InstMesh(const Mesh& mesh, const size_t numInsts, const size_t baseIndex, const std::function<void(GLattrarr&)>& attrs) {
        GLattrarr attrSetup;

        vao.create();
        vao.bind();
        verts.create(GL_ARRAY_BUFFER);
        insts.create(GL_ARRAY_BUFFER, GL_STREAM_DRAW);
        elems.create(GL_ELEMENT_ARRAY_BUFFER);

        auto& meshIndices = mesh.indices().verts;
        auto& meshVerts = mesh.data().verts;

        elems.bind();
        numVerts = meshIndices.size();
        elems.data(sizeof(GLuint) * numVerts, meshIndices.data());

        verts.bind();
        verts.data(sizeof(vec3) * meshVerts.size(), meshVerts.data());

        attrSetup.add<vec3>(1);
        attrSetup.apply();

        insts.bind();
        insts.data(sizeof(T) * numInsts, nullptr);
        instances.reserve(numInsts);

        attrs(attrSetup);
        attrSetup.apply(baseIndex);
    }
    
    inline void update() const {
        insts.bind();
        insts.data(instances.data());
    }
    inline void draw(Render::MaterialPass* renderer, Render::Info* mat, const size_t group) const {
        renderer->scheduleDrawElements(group, nullptr, &vao, mat, GL_TRIANGLES, numVerts, GLtype<uint32_t>(), instances.size());
    }

private:
    size_t numVerts;
    GLVAO vao;
    GLbuffer verts, elems, insts;
};

class DrawDebug {
public:
    static DrawDebug& get();

    void flush();
    void postUpdate();

    //this is the actual draw call
    void draw(Render::MaterialPass* deferred, Render::MaterialPass* forward);

    //these are called externally for drawing stuff
    void drawDebugVector(vec3 start, vec3 end, vec3 color = vec3(0.7f, 1, 0));
    void drawDebugSphere(vec3 pos, float rad, vec3 color = vec3(0.8f, 0.7f, 1.f), float opacity = 0.3f);
    void drawDebugBox(vec3 pos, float l, vec3 color = vec3(1.f), float opacity = 1.f) { drawDebugBox(pos, l, l, l, color, opacity); };
    void drawDebugBox(vec3 pos, float w, float h, float d, vec3 color = vec3(1.f), float opacity = 1.f);

    uint32_t fillIndex, wireframeIndex;
private:
    DrawDebug();
    DrawDebug(const DrawDebug&) = delete;
    void operator=(const DrawDebug&) = delete;

    //these are to separate the individual processes
    void drawVectors();
    void drawSpheres();
    void drawBoxes();

    shared<Mesh> arrow, sphere, box;

    void setRenderers(Render::MaterialPass* deferred, Render::MaterialPass* forward);
    Render::MaterialPass* deferred, * forward;
    Render::Info vecMat, meshMat;
    
    struct MeshData { vec4 color; mat4 transform; };

    GLVAO vecVAO;
    GLbuffer vecBuffer;

    struct Vector {
        struct Point { vec3 pos, color; };
        Point start, end;
    };
    struct Box { vec3 pos, dims; vec4 color; };
    struct Sphere { vec4 color; vec3 center; float rad; };

    InstMesh<MeshData> arrows, spheres, boxes;
    std::vector<Vector> vectorInsts;
    std::atomic<size_t> vecsAdded, spheresAdded, boxesAdded;

    thread_frame_vector<Vector> debugVectors;
    thread_frame_vector<Sphere> debugSpheres;
    thread_frame_vector<Box> debugBoxes;
};