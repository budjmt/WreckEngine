#include "Light.h"

#include "ModelHelper.h"

using namespace Light;

size_t Point::count;
size_t Spotlight::count;

void Point::bindGeometryImpl() {
    struct GeometrySetup {
        GLbuffer verts, elems;
        GeometrySetup() {
            auto mesh = loadOBJ("Assets/Lights/point.obj");

            verts.create(GL_ARRAY_BUFFER);
            elems.create(GL_ELEMENT_ARRAY_BUFFER);

            verts.bind();
            verts.data(sizeof(vec3) * mesh->data().verts.size(), &mesh->data().verts[0]);

            Point::count = mesh->indices().verts.size();
            elems.bind();
            elems.data(sizeof(GLuint) * Point::count, &mesh->indices().verts[0]);
        }
    };

    static GeometrySetup geometry;
    geometry.verts.bind();
    geometry.elems.bind();
}

void Directional::bindGeometryImpl() {
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
            clockwiseFSTri.data(sizeof(verts), &verts);
        }
    };

    static GeometrySetup geometry;
    geometry.clockwiseFSTri.bind();
}

void Spotlight::bindGeometryImpl() {
    struct GeometrySetup {
        GLbuffer verts, elems;
        GeometrySetup() {
            auto mesh = loadOBJ("Assets/Lights/spotlight.obj");

            verts.create(GL_ARRAY_BUFFER);
            elems.create(GL_ELEMENT_ARRAY_BUFFER);

            verts.bind();
            verts.data(sizeof(vec3) * mesh->data().verts.size(), &mesh->data().verts[0]);

            Spotlight::count = mesh->indices().verts.size();
            elems.bind();
            elems.data(sizeof(GLuint) * Spotlight::count, &mesh->indices().verts[0]);
        }
    };

    static GeometrySetup geometry;
    geometry.verts.bind();
    geometry.elems.bind();
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
    static GLprogram prog = getLightProg("Shaders/light/" #name "_v.glsl", "Shaders/light/" #name "_f.glsl"); \
    static GLuniform<mat4> camLoc = prog.getUniform<mat4>("camera"); \
    static GLuniform<vec3> camPosLoc = prog.getUniform<vec3>("camPos"); \
    static GLresource<GLresolution> resolutionLoc = GLresource<GLresolution>(prog, "resolution"); \
    Manager<type> m; \
    m.camMat = camLoc; \
    m.camPos = camPosLoc; \
    m.renderInfo.setShaders(prog, &resolutionLoc); \
    m.renderInfo.setTextures(Render::gBuffer[1], Render::gBuffer[2]); \
    return m; \
 }

MAKE_MANAGER(Point, point)
MAKE_MANAGER(Directional, directional)
MAKE_MANAGER(Spotlight, spotlight)
