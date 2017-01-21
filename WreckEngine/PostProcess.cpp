#include "PostProcess.h"

#include "ShaderHelper.h"
#include "HotSwap.h"

#include "Render.h"

using namespace Render;

HotSwap::Resource<File::Extension::GLSL> defaultVertex;

PostProcess::PostProcess() { fbo.create(); }

void PostProcess::init() {
    defaultVertex = decltype(defaultVertex)("Shaders/postProcess/res_v.glsl", GL_VERTEX_SHADER);
}

GLprogram PostProcess::make_program(const char* shaderFile) { return make_program(loadShader(shaderFile, GL_FRAGMENT_SHADER), shaderFile); }
GLprogram PostProcess::make_program(const GLshader& fragment, const char* path) {
    assert(fragment.type == GL_FRAGMENT_SHADER);
    auto prog = HotSwap::Shader::create();
    prog->vertex = defaultVertex;
    prog->fragment.set(fragment, path, GL_FRAGMENT_SHADER);
    prog->setupProgram();
    return prog->getProgram();
}

PostProcess* PostProcess::chainsTo(shared<PostProcess> p) { 
    chain.push_back(p); 
    return p.get(); 
}

PostProcess* PostProcess::chainsTo(shared<Composite> p) { 
    chain.push_back(p); 
    p->dependencies.push_back(this); 
    return p.get(); 
}

void PostProcess::apply() {
    if (!fbo.isBound()) fbo.bind();
    GLframebuffer::clear();
    data.apply();
    GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, 3));
    renderer->finish(this);
}

PostProcess* PostProcess::cyclesWith(size_t numCycles, shared<PostProcess> p) {
    assert(numCycles > 1); // cycles of < 1 are errors, cycles of 1 are a normal progression
    assert(chain.empty() && p->chain.empty()); // only chain-less PPs can cycle
    
    auto q = this;                              // q starts as the caller
    q->chainsTo(p);                             // q chain = {p}
    while (--numCycles) {
        auto qc = make_shared<PostProcess>(*q); // 1st iteration: copy of q, chain = {p}
        auto pc = new PostProcess(*p);          // copy of p, chain = {}
        qc->chain.clear();                      // now q copy chain = {}, q still = {p}
        q = p->chainsTo(qc);                    // q now points to q copy, p's chain = {q copy} (never chain to original object), original is in p
        p.reset(pc);                            // p now points to its copy, chain = {}, original p still in q's chain
        q->chainsTo(p);                         // q copy chain = {p copy}
    }
    return p.get(); // the actual end of the chain, some copy of the original p
}