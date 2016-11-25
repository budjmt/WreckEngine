#include "Render.h"

#include "External.h"

#include "ShaderHelper.h"

using namespace Render;

MaterialRenderer::MaterialRenderer(const size_t gBufferSize) {
    frameBuffer.create();
    frameBuffer.attachTexture(depthstencil, GLframebuffer::Attachment::DepthStencil);

    for (size_t i = 0; i < gBufferSize; ++i)
        frameBuffer.attachTexture(gBuffer[i], GLframebuffer::Attachment::Color);
}

std::vector<GLtexture> MaterialRenderer::gBuffer;
GLtexture MaterialRenderer::depthstencil;

void MaterialRenderer::init(const size_t max_gBufferSize) {
    depthstencil = GLframebuffer::createRenderTarget<GLubyte>(GL_DEPTH24_STENCIL8);
    
    gBuffer.reserve(max_gBufferSize);
    for (size_t i = 0; i < max_gBufferSize; ++i) {
        gBuffer.push_back(GLframebuffer::createRenderTarget<GLubyte>());
    }
}

PostProcessRenderer::PostProcessRenderer() {
    triangle.create();
    triangle.bind();

    GLbuffer buffer;
    buffer.create(GL_ARRAY_BUFFER);
    buffer.bind();

    const float verts[] = {
        0.f, 0.f,
        2.f, 0.f,
        0.f, 2.f
    };
    buffer.data(sizeof(verts), &verts);

    GLattrarr attr;
    attr.add<vec2>(1);
    attr.apply();

    triangle.unbind();

    output = GLframebuffer::createRenderTarget<GLubyte>();
}

GLprogram PostProcessRenderer::finalize;

void PostProcessRenderer::init() {
    PostProcess::init();
    finalize.create();
    finalize.vertex = PostProcess::defaultVertex;
    finalize.fragment = loadShader("Shaders/postprocess_final_f.glsl", GL_FRAGMENT_SHADER);
    finalize.link();
    finalize.setOnce<GLsampler>("render", 0);
}

void MaterialRenderer::scheduleDraw(const DrawCall d) { drawCalls.push_back(d); }

void MaterialRenderer::scheduleDrawArrays(const GLVAO* vao, const Info* mat, const GLenum tesselPrim, const uint32_t count, const uint32_t instances) {
    DrawCall d;
    d.vao = vao;
    d.material = mat;
    d.call = DrawCall::Type::Arrays;
    d.tesselPrim = tesselPrim;
    d.params.count = count;
    d.params.instances = instances;
    scheduleDraw(d);
}

void MaterialRenderer::scheduleDrawElements(const GLVAO* vao, const Info* mat, const GLenum tesselPrim, const uint32_t count, const GLenum element_t, const uint32_t instances) {
    DrawCall d;
    d.vao = vao;
    d.material = mat;
    d.call = DrawCall::Type::Elements;
    d.tesselPrim = tesselPrim;
    d.element_t = element_t;
    d.params.count = count;
    d.params.instances = instances;
    scheduleDraw(d);
}

void MaterialRenderer::render() {
    frameBuffer.bind();
    for (const auto& drawCall : drawCalls) drawCall.render();
    drawCalls.clear();
}

void PostProcessRenderer::apply() {
    triangle.bind();
    finish(&entry);
    entry.reset(); // resets the whole chain
}

// performs necessary steps after a post-process completes
void PostProcessRenderer::finish(PostProcess* curr) {
    if (curr->chain.empty()) {
        curr->fbo.unbind();
        finalize.use();
        output.bind();
        // output the final texture to the screen
        GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, 3));
    }
    else {
        for (auto p : curr->chain) p->apply(curr);
    }
}