#include "PostProcess.h"

#include "ShaderHelper.h"

#include "Render.h"

using namespace Render;

GLshader PostProcess::defaultVertex;

void PostProcess::init() {
    defaultVertex = loadShader("Shaders/postprocess_v.glsl", GL_VERTEX_SHADER);
}

void PostProcess::chainsTo(PostProcess* p) { chain.push_back(p); }
void PostProcess::chainsTo(Composite* p) { chain.push_back(p); p->dependencies.push_back(this); }

void PostProcess::apply(PostProcess* prev) {
    if (!fbo.isBound()) fbo.bind();
    data.apply();
    GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, 3));
    renderer->finish(this);
}