#include "Light.h"

#include "ModelHelper.h"

using namespace Light;

size_t Point::count;
size_t Spotlight::count;

GLuint Point::setupGeometryImpl(GLattrarr& attrs) {

    struct GeometrySetup {
        GLbuffer verts, elems;
        GeometrySetup(const char* file, size_t& count) {
            auto mesh = loadOBJ(file);

            verts.create(GL_ARRAY_BUFFER);
            elems.create(GL_ELEMENT_ARRAY_BUFFER);

            verts.bind();
            verts.data(sizeof(vec3) * mesh->data().verts.size(), mesh->data().verts.data());

            count = mesh->indices().verts.size();
            elems.bind();
            elems.data(sizeof(GLuint) * count, mesh->indices().verts.data());
        }
    };

    static GeometrySetup geometry ("Assets/Lights/point.obj", Point::count);
    geometry.verts.bind();
    geometry.elems.bind();
    attrs.add<vec3>(1);
    return attrs.apply();
}

GLuint Directional::setupGeometryImpl(GLattrarr& attrs) {
    struct GeometrySetup {
        GLbuffer clockwiseFSTri;
        GeometrySetup() {
            clockwiseFSTri.create(GL_ARRAY_BUFFER);
            clockwiseFSTri.bind();

            constexpr float verts[] = {
                -1.f, -1.f, 0.f, 0.f,
                -1.f,  3.f, 0.f, 2.f,
                 3.f, -1.f, 2.f, 0.f
            };
            clockwiseFSTri.data(sizeof(verts), verts);
        }
    };

    static GeometrySetup geometry;
    geometry.clockwiseFSTri.bind();
    attrs.add<vec2>(2);
    return attrs.apply();
}

GLuint Spotlight::setupGeometryImpl(GLattrarr& attrs) {

    struct GeometrySetup {
        GLbuffer verts, elems;
        GeometrySetup(const char* file, size_t& count) {
            auto mesh = loadOBJ(file);
            mesh->translate({ 0, -0.5f, 0 });

            verts.create(GL_ARRAY_BUFFER);
            elems.create(GL_ELEMENT_ARRAY_BUFFER);

            verts.bind();
            verts.data(sizeof(vec3) * mesh->data().verts.size(), mesh->data().verts.data());

            count = mesh->indices().verts.size();
            elems.bind();
            elems.data(sizeof(GLuint) * count, mesh->indices().verts.data());
        }
    };

    static GeometrySetup geometry ("Assets/Lights/spotlight.obj", Spotlight::count);
    geometry.verts.bind();
    geometry.elems.bind();
    attrs.add<vec3>(1);
    return attrs.apply();
}

#include "Render.h"

bool Directional::isTransformed(const Directional& update) const {
    return false;
}

Directional::DeferredData Directional::getDeferredData() const {
    DeferredData data;
    auto& camData = Render::Renderer::getCamData();
    auto dir = camData.forward;
    auto pos = camData.position - dir * 2.f;
    data.lightWorldViewProj = glm::translate(pos) * glm::lookAt(pos, pos + dir, { 0, 1, 0 }) * camData.cam->projection();
    return data;
}

#include "ShaderHelper.h"

GLprogram getLightProg(const char* vs, const char* fs) {
    auto prog = loadProgram(vs, fs);
    prog.use();
    prog.setOnce<GLsampler>("gPosition", 0);
    prog.setOnce<GLsampler>("gNormal", 1);
    return prog;
}

#define MAKE_MANAGER(type, name) \
Manager<type> Light::make_manager_ ## name () { \
    struct X { \
        Render::Info material; \
        X() { \
            auto prog = getLightProg("Shaders/light/" #name "_v.glsl", "Shaders/light/" #name "_f.glsl"); \
            material.setShaders(prog); \
            material.addResource<GLcamera::matrix>("camera"); \
            material.addResource<GLcamera::position>("camPos"); \
            material.addResource<GLresolution>("resolution"); \
            material.setTextures(Render::gBuffer[0], Render::gBuffer[1]); \
        } \
    }; \
    static X managerMat; \
    Manager<type> m; \
    m.renderInfo = managerMat.material; \
    return m; \
 }

MAKE_MANAGER(Point, point)
MAKE_MANAGER(Directional, directional)
MAKE_MANAGER(Spotlight, spotlight)
