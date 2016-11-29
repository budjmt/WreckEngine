#include "Render.h"

#include "External.h"

#include "ShaderHelper.h"

using namespace Render;

MaterialRenderer::MaterialRenderer(const size_t gBufferSize) {
    frameBuffer.create();
    frameBuffer.bindPartial();
    frameBuffer.attachTexture(depth, GLframebuffer::Attachment::DepthStencil);
    //frameBuffer.attachTexture(stencil, GLframebuffer::Attachment::Stencil);

    assert(gBufferSize <= gBuffer.size());

    for (size_t i = 0; i < gBufferSize; ++i)
        frameBuffer.attachTexture(gBuffer[i], GLframebuffer::Attachment::Color);

    frameBuffer.unbind();
}

std::vector<GLtexture> MaterialRenderer::gBuffer;
GLtexture MaterialRenderer::depth;
GLtexture MaterialRenderer::stencil;
GLtexture MaterialRenderer::prevOutput;

void MaterialRenderer::init(const size_t max_gBufferSize) {
    // these can be optimized in 4.3+ using texture views
    depth = GLframebuffer::createRenderTarget<GLdepthstencil>(GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL);
    // this is more labor intensive than expected
    //stencil = GLframebuffer::createRenderTarget<GLdepthstencil>(GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL);
    //stencil.param(GL_DEPTH_STENCIL_TEXTURE_MODE, GL_STENCIL_INDEX);
    
    gBuffer.reserve(max_gBufferSize);
    for (size_t i = 0; i < max_gBufferSize; ++i) {
        gBuffer.push_back(GLframebuffer::createRenderTarget<GLubyte>());
    }
}

PostProcessRenderer::PostProcessRenderer() {
    output = GLframebuffer::createRenderTarget<GLubyte>();
}

GLVAO PostProcessRenderer::triangle;
GLprogram PostProcessRenderer::finalize;

void PostProcessRenderer::init() {
    PostProcess::init();
    finalize.create();
    finalize.vertex = PostProcess::defaultVertex;
    finalize.fragment = loadShader("Shaders/postprocess_final_f.glsl", GL_FRAGMENT_SHADER);
    finalize.link();
    finalize.use();
    finalize.setOnce<GLsampler>("render", 0);

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
}

void MaterialRenderer::scheduleDraw(const size_t group, const DrawCall d) { 
    assert(renderGroups.size() > group);
    renderGroups[group].drawCalls.push_back(d); 
}

void MaterialRenderer::scheduleDrawArrays(const size_t group, const GLVAO* vao, const Info* mat, const GLenum tesselPrim, const uint32_t count, const uint32_t instances) {
    DrawCall d;
    d.vao = vao;
    d.material = mat;
    d.call = DrawCall::Type::Arrays;
    d.tesselPrim = tesselPrim;
    d.params.count = count;
    d.params.instances = instances;
    scheduleDraw(group, d);
}

void MaterialRenderer::scheduleDrawElements(const size_t group, const GLVAO* vao, const Info* mat, const GLenum tesselPrim, const uint32_t count, const GLenum element_t, const uint32_t instances) {
    DrawCall d;
    d.vao = vao;
    d.material = mat;
    d.call = DrawCall::Type::Elements;
    d.tesselPrim = tesselPrim;
    d.element_t = element_t;
    d.params.count = count;
    d.params.instances = instances;
    scheduleDraw(group, d);
}

void MaterialRenderer::render() {
    frameBuffer.bind();
    for (auto& renderGroup : renderGroups) {
        Group::Helper(renderGroup).draw();
    }
}

void PostProcessRenderer::apply() {
    triangle.bind();
    finish(&entry);
    entry.reset(); // resets the whole chain
}

// performs necessary steps after a post-process completes
void PostProcessRenderer::finish(PostProcess* curr) {
    if (!curr->endsChain()) {
        for (auto p : curr->chain) p->apply(curr);
    }
}

// output the final texture to the screen
void PostProcessRenderer::render() const {
    GLframebuffer::unbind(GL_FRAMEBUFFER);
    finalize.use();
    output.bind();
    GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, 3));
}