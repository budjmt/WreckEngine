#include "Render.h"

#include "External.h"

#include "ShaderHelper.h"
#include "Camera.h"

#include "GLstate.h"

using namespace Render;

CameraData::CameraData(Camera* c) : cam(c) {
    if (!cam) return;
    viewProjection = cam->getCamMat();
    {
        auto t = cam->transform.getComputed();
        position = t->position();
        forward = t->forward();
    }
}

Event::Handler Target::resizeHandler = Event::make_handler<Window::ResizeHandler>(&resizeTargets);
std::vector<Target> Target::targets;

void Target::resizeTargets(Event::Handler::param_t e) {
    Thread::Render::runNextFrame([] {
        for (auto& target : targets) {
            auto formatInfo = target.formatInfo;
            target.texture.bind();
            target.texture.set2D(formatInfo.type, nullptr, Window::frameWidth, Window::frameHeight, formatInfo.from, formatInfo.to);
        }
    });
}

std::vector<GLtexture> Render::gBuffer;
GLtexture Render::depthstencil;

GLbuffer Render::fs_triangle;

void Renderer::init(const size_t max_gBufferSize) {
    // only depth OR stencil can be sampled without texture views (which only work on immutable storage)
    // to change mode: depthstencil.param(GL_DEPTH_STENCIL_TEXTURE_MODE, GL_DEPTH_COMPONENT/GL_STENCIL_COMPONENT);
    depthstencil = Target::create<GLdepthstencil>(GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL);

    gBuffer.reserve(max_gBufferSize);
    for (size_t i = 0; i < max_gBufferSize; ++i) {
        gBuffer.push_back(Target::create<GLfloat>(GL_RGBA16F));
    }

    fs_triangle.create(GL_ARRAY_BUFFER);
    fs_triangle.bind();

    constexpr float verts[] = {
        -1.f, -1.f, 0.f, 0.f,
         3.f, -1.f, 2.f, 0.f,
        -1.f,  3.f, 0.f, 2.f
    };
    fs_triangle.data(sizeof(verts), &verts);

    fs_triangle.unbind();

    PostProcessChain::init();
}

void MaterialPass::prepareFrameBuffer() {
    frameBuffer.create();
    frameBuffer.bindPartial();
    frameBuffer.attachTexture(depthstencil, GLframebuffer::Attachment::DepthStencil);
    //frameBuffer.attachTexture(stencil, GLframebuffer::Attachment::Stencil);
}

MaterialPass::MaterialPass(const size_t gBufferSize) {
    prepareFrameBuffer();

    assert(gBufferSize <= gBuffer.size());

    for (size_t i = 0; i < gBufferSize; ++i)
        frameBuffer.attachTexture(gBuffer[i], GLframebuffer::Attachment::Color);

    frameBuffer.unbind();
}

MaterialPass::MaterialPass(const std::vector<GLuint>& targets) {
    prepareFrameBuffer();

    assert(targets.size() <= gBuffer.size());

    for (auto target : targets) {
        assert(target >= 0 && target < gBuffer.size());
        frameBuffer.attachTexture(gBuffer[target], GLframebuffer::Attachment::Color);
    }

    frameBuffer.unbind();
}

MaterialPass::MaterialPass(const std::vector<GLtexture>& colorTargets, const GLtexture& depthstencilTarget) {
    frameBuffer.create();
    frameBuffer.bindPartial();
    frameBuffer.attachTexture(depthstencilTarget, GLframebuffer::Attachment::DepthStencil);

    for (const auto& target : colorTargets) {
        frameBuffer.attachTexture(target, GLframebuffer::Attachment::Color);
    }

    frameBuffer.unbind();
}

GLVAO PostProcessChain::triangle;
GLprogram PostProcessChain::finalize;

void PostProcessChain::init() {
    PostProcess::init();
    finalize = PostProcess::make_program("Shaders/postProcess/res_final_f.glsl");
    finalize.use();
    finalize.setOnce<GLsampler>("render", 0);

    triangle.create();
    triangle.bind();

    fs_triangle.bind();
    GLattrarr attr;
    attr.add<vec2>(2);
    attr.apply();

    triangle.unbind();
}

void MaterialPass::scheduleDraw(size_t group, DrawCall d, DrawCall::Params p) { 
    assert(renderGroups.size() > group);
    auto& renderGroup = renderGroups[group];
    renderGroup.drawCalls.push_back({ d, p });
}

void MaterialPass::scheduleDrawArrays(size_t group, const Entity* entity, const GLVAO* vao, const Info* mat, GLenum tesselPrim, uint32_t count, uint32_t instances) {
    DrawCall d;
    d.entity = entity;
    d.vao = vao;
    d.material = mat;
    d.call = DrawCall::Type::Arrays;
    d.tesselPrim = tesselPrim;

    DrawCall::Params params{};
    params.count = count;
    params.instances = instances;

    scheduleDraw(group, d, params);
}

void MaterialPass::scheduleDrawElements(const size_t group, const Entity* entity, const GLVAO* vao, const Info* mat, const GLenum tesselPrim, const uint32_t count, const GLenum element_t, const uint32_t instances) {
    DrawCall d;
    d.entity = entity;
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

void MaterialPass::render() {
    frameBuffer.bind();
    for (auto& renderGroup : renderGroups) {
        Group::Helper(renderGroup).draw();
    }
}

void MaterialPass::Group::Helper::draw() {
    auto& drawCalls = group.drawCalls;
    if (drawCalls.empty())
        return;

    // copy the raw draw params to the buffer
    auto& rawDrawParams = group.rawDrawParams;
    rawDrawParams.reserve(drawCalls.size());
    for (auto& drawCall : drawCalls)
        rawDrawParams.push_back(drawCall.params);

    auto& paramBuffer = group.paramBuffer;
    paramBuffer.bind();
    paramBuffer.invalidate();
    paramBuffer.data(rawDrawParams.size() * sizeof(DrawCall::Params), &rawDrawParams[0]);
    // possible synchronization issue when param buffer does not complete upload before draw call?

    DrawCall::Params* offset = nullptr; // &params[0];
    for (const auto& drawCall : drawCalls) {

        const auto& prog = drawCall.data.material->shaders->program;
        assert(!prog.tessControl.valid() || drawCall.data.tesselPrim == GL_PATCHES); // if using tessellation, the primitive type must be GL_PATCHES
        assert(!prog.isCompute); // compute shaders may not be used for draw calls

        drawCall.data.render(offset);
        ++offset;
    }

    drawCalls.clear();
    rawDrawParams.clear();
}

void PostProcessChain::apply() {
    GLstate<GL_ENABLE_BIT, GL_DEPTH_TEST> depthSave;
    GLstate<GL_ENABLE_BIT, GL_CULL_FACE> cullSave;
    GL_CHECK(glDisable(GL_DEPTH_TEST));
    GL_CHECK(glDisable(GL_CULL_FACE));
    triangle.bind();
    finish(&entry);
    entry.refresh(); // resets the whole chain
}

// performs necessary steps after a post-process completes
void PostProcessChain::finish(PostProcess* curr) {
    if (!curr->endsChain()) {
        for (auto& p : curr->chain) 
            p->apply();
    }
}

// output the final texture to the screen
void PostProcessChain::render() const {
    GLstate<GL_ENABLE_BIT, GL_DEPTH_TEST> depthSave;
    GLstate<GL_ENABLE_BIT, GL_BLEND> blendSave;
    GLstate<GL_ENABLE_BIT, GL_CULL_FACE> cullSave;
    GL_CHECK(glDisable(GL_DEPTH_TEST));
    GL_CHECK(glDisable(GL_BLEND));
    GL_CHECK(glDisable(GL_CULL_FACE));

    GLframebuffer::unbind(GL_FRAMEBUFFER);
    finalize.use();
    (entry.endsChain() ? gBuffer[0] : output).bind();
    GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, 3));
}

static CameraData currRenderCamData;
const CameraData& Renderer::getCamData() { return currRenderCamData; }

static GLframebuffer genClearFB() {
    GLframebuffer f;
    f.create();
    f.bindPartial();
    for (size_t i = 0, gBufferSize = gBuffer.size(); i < gBufferSize; ++i)
        f.attachTexture(gBuffer[i], GLframebuffer::Attachment::Color);
    f.attachTexture(depthstencil, GLframebuffer::Attachment::DepthStencil);
    return f;
}

static vec4 clearPrevFrame(size_t clearIndex) {
    static GLframebuffer clearFrame = genClearFB();
    
    GLstate<GL_DEPTH, GL_DEPTH_WRITEMASK> maskState;
    GL_CHECK(glDepthMask(GL_TRUE));

    auto color = GLframebuffer::getClearColor();
    GLframebuffer::setClearColor(0.f, 0.f, 0.f, 0.f);

    clearFrame.bind();
    GLframebuffer::clear();
    
    GLframebuffer::setClearColor(color.r, color.g, color.b, color.a);
    GLframebuffer::clearPartial(GL_COLOR, clearIndex, &color[0]); // clears color to original clear color

    GLframebuffer::setClearColor(0.f, 0.f, 0.f, 0.f);
    return color;
}

void Renderer::render() {
    auto clearColor = clearPrevFrame(clearColorIndex);
    renderChildren();
    GLframebuffer::setClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
}

void Renderer::renderChildren() {
    currRenderCamData = CameraData(renderCam ? renderCam : Camera::main); // happens before setup for any dependent code
    setup();
    objects.render();
    postProcess.apply();
    if (next) {
        next->renderChildren();
    }
    else postProcess.render();
}