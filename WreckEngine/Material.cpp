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

void DrawCall::render() const {
    vao->bind();
    material->apply();

    switch (call) {
    case Type::Arrays:
        glDrawArraysIndirect(tesselPrim, &params);
        break;
    case Type::Elements:
        glDrawElementsIndirect(tesselPrim, element_t, &params);
        break;
    }
}