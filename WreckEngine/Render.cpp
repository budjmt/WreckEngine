#include "Render.h"

#include "External.h"

#include "ShaderHelper.h"

using namespace Render;

std::vector<GLtexture> Render::gBuffer;
GLtexture Render::depth, Render::stencil;
GLtexture Render::prevOutput;

void FullRenderer::init(const size_t max_gBufferSize) {
    // these can be optimized in 4.3+ using texture views
    depth = GLframebuffer::createRenderTarget<GLdepthstencil>(GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL);
    // this is more labor intensive than expected
    //stencil = GLframebuffer::createRenderTarget<GLdepthstencil>(GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL);
    //stencil.param(GL_DEPTH_STENCIL_TEXTURE_MODE, GL_STENCIL_INDEX);

    gBuffer.reserve(max_gBufferSize);
    for (size_t i = 0; i < max_gBufferSize; ++i) {
        gBuffer.push_back(GLframebuffer::createRenderTarget<GLubyte>());
    }

    PostProcessRenderer::init();
}

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

PostProcessRenderer::PostProcessRenderer() {
    output = GLframebuffer::createRenderTarget<GLubyte>();
}

GLVAO PostProcessRenderer::triangle;
GLprogram PostProcessRenderer::finalize;

void PostProcessRenderer::init() {
    PostProcess::init();
    finalize = PostProcess::make_program(loadShader("Shaders/postProcess/res_final_f.glsl", GL_FRAGMENT_SHADER));
    finalize.use();
    finalize.setOnce<GLsampler>("render", 0);

    triangle.create();
    triangle.bind();

    GLbuffer buffer;
    buffer.create(GL_ARRAY_BUFFER);
    buffer.bind();

    constexpr float verts[] = {
        -1.f, -1.f, 0.f, 0.f,
         3.f, -1.f, 2.f, 0.f,
        -1.f,  3.f, 0.f, 2.f
    };
    buffer.data(sizeof(verts), &verts);

    GLattrarr attr;
    attr.add<vec2>(2);
    attr.apply();

    triangle.unbind();
}

void MaterialRenderer::scheduleDraw(const size_t group, const DrawCall d, const DrawCall::Params p) { 
    assert(renderGroups.size() > group);
    auto& renderGroup = renderGroups[group];
    renderGroup.drawCalls.push_back(d); 
    renderGroup.params.push_back(p);
}

void MaterialRenderer::scheduleDrawArrays(const size_t group, const GLVAO* vao, const Info* mat, const GLenum tesselPrim, const uint32_t count, const uint32_t instances) {
    DrawCall d;
    d.vao = vao;
    d.material = mat;
    d.call = DrawCall::Type::Arrays;
    d.tesselPrim = tesselPrim;
    
    DrawCall::Params params{};
    params.count = count;
    params.instances = instances;
    
    scheduleDraw(group, d, params);
}

void MaterialRenderer::scheduleDrawElements(const size_t group, const GLVAO* vao, const Info* mat, const GLenum tesselPrim, const uint32_t count, const GLenum element_t, const uint32_t instances) {
    DrawCall d;
    d.vao = vao;
    d.material = mat;
    d.call = DrawCall::Type::Elements;
    d.tesselPrim = tesselPrim;
    d.element_t = element_t;

    DrawCall::Params params{};
    params.count = count;
    params.instances = instances;
    
    scheduleDraw(group, d, params);
}

void MaterialRenderer::render() {
    frameBuffer.bind();
    GLframebuffer::clear();

    for (auto& renderGroup : renderGroups) {
        Group::Helper(renderGroup).draw();
    }
}

void MaterialRenderer::Group::Helper::draw() {
    auto& drawCalls = group.drawCalls;
    if (drawCalls.empty())
        return;

    auto& params = group.params;
    assert(drawCalls.size() == params.size());

    auto& paramBuffer = group.paramBuffer;
    paramBuffer.bind();
    paramBuffer.invalidate();
    paramBuffer.data(group.params.size() * sizeof(DrawCall::Params), &params[0]);

    DrawCall::Params* offset = nullptr;
    for (const auto& drawCall : drawCalls) {
        drawCall.render(offset);
        ++offset;
    }

    group.drawCalls.clear();
    group.params.clear();
}

void PostProcessRenderer::apply() {
    triangle.bind();
    finish(&entry);
    entry.refresh(); // resets the whole chain
}

// performs necessary steps after a post-process completes
void PostProcessRenderer::finish(PostProcess* curr) {
    if (!curr->endsChain()) {
        for (auto& p : curr->chain) p->apply(curr);
    }
}

// output the final texture to the screen
void PostProcessRenderer::render() const {
    GLframebuffer::unbind(GL_FRAMEBUFFER);
    finalize.use();
    (entry.endsChain() ? gBuffer[0] : output).bind();
    GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, 3));
}