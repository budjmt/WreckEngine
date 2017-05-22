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
    InstMesh(const Mesh* mesh, const size_t numInsts, const size_t baseIndex, const std::function<void(GLattrarr&)>& attrs) {
        GLattrarr attrSetup;

        vao.create();
        vao.bind();
        verts.create(GL_ARRAY_BUFFER);
        insts.create(GL_ARRAY_BUFFER, GL_STREAM_DRAW);
        elems.create(GL_ELEMENT_ARRAY_BUFFER);

        elems.bind();
        numVerts = mesh->indices().verts.size();
        elems.data(sizeof(GLuint) * numVerts, &mesh->indices().verts[0]);

        verts.bind();
        verts.data(sizeof(vec3) * mesh->data().verts.size(), &mesh->data().verts[0]);

        attrSetup.add<vec3>(1);
        attrSetup.apply();

        insts.bind();
        insts.data(sizeof(T) * numInsts, nullptr);
        instances.reserve(numInsts);

        attrs(attrSetup);
        attrSetup.apply(baseIndex);
    }
    
    inline void update() const {
        vao.bind();
        insts.bind();
        insts.data(instances.data());
    }
    inline void draw(Render::MaterialPass* renderer, Render::Info* mat, const size_t group) const {
        renderer->scheduleDrawElements(group, &vao, mat, GL_TRIANGLES, numVerts, GLtype<uint32_t>(), instances.size());
    }

private:
    size_t numVerts;
    GLVAO vao;
    GLbuffer verts, elems, insts;
};

class DrawDebug {
public:
    static DrawDebug& getInstance();
    void camera(Camera* c);

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

    Camera* cam = nullptr;
    GLuniform<mat4> vecCam, meshCam;

    shared<Mesh> arrow, sphere, box;

    void setRenderers(Render::MaterialPass* deferred, Render::MaterialPass* forward);
    Render::MaterialPass* deferred, * forward;
    Render::Info vecMat, meshMat;
    
    struct m_MeshData { vec4 color; mat4 transform; };

    GLVAO vecVAO;
    GLbuffer vecBuffer;
    
    InstMesh<m_MeshData> arrows, spheres, boxes;
    std::vector<vec3> vectorInsts;
    std::atomic<size_t> vecsAdded, spheresAdded, boxesAdded;
    
    struct Sphere { vec4 color; vec3 center; float rad; };

    thread_frame_vector<vec3> debugVectors;
    thread_frame_vector<vec4> debugBoxes;
    thread_frame_vector<Sphere> debugSpheres;
};