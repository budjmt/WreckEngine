#include "Material.h"

using namespace Render;

void Info::ShaderBinding::update() const {
    program.use();
    for (auto resource : resources)
        resource->update();
}

void Info::TextureBinding::bind() const {
    for (size_t i = 0, numTextures = bindings.size(); i < numTextures; ++i)
        bindings[i].bind(i);
}

void DrawCall::render(const void* offset) const {
    vao->bind();
    material->apply();

    auto p = (Params*) offset;
    switch (call) {
    case Type::Arrays:
        //GL_CHECK(glDrawArraysIndirect(tesselPrim, offset));
        GL_CHECK(glDrawArraysInstanced(tesselPrim, p->first, p->count, p->instances));
        break;
    case Type::Elements:
        //GL_CHECK(glDrawElementsIndirect(tesselPrim, element_t, offset));
        GL_CHECK(glDrawElementsInstanced(tesselPrim, p->count, element_t, nullptr, p->instances));
        break;
    }
}