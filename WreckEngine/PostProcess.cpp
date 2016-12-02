#include "PostProcess.h"

#include "ShaderHelper.h"

#include "Render.h"

using namespace Render;

GLshader PostProcess::defaultVertex;

PostProcess::PostProcess() { fbo.create(); }

void PostProcess::init() {
    defaultVertex = loadShader("Shaders/postProcess/res_v.glsl", GL_VERTEX_SHADER);
}

GLprogram PostProcess::make_program(const char* shaderFile) { return make_program(loadShader(shaderFile, GL_FRAGMENT_SHADER)); }
GLprogram PostProcess::make_program(const GLshader& fragment) {
    assert(fragment.type == GL_FRAGMENT_SHADER);
    GLprogram p;
    p.vertex = defaultVertex;
    p.fragment = fragment;
    p.create();
    p.link();
    return p;
}

void PostProcess::chainsTo(PostProcess p) { chain.push_back(p); }
void PostProcess::chainsTo(Composite p) { chain.push_back(p); p.dependencies.push_back(this); }

void PostProcess::apply(PostProcess* prev) {
    if (!fbo.isBound()) fbo.bind();
    GLframebuffer::clear();
    data.apply();
    GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, 3));
    renderer->finish(this);
}