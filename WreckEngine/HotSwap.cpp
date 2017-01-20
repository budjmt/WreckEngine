#include "HotSwap.h"

using namespace HotSwap;

std::vector<shared<SwapResource>> HotSwap::resources;

void HotSwap::main() {
    for (auto& resource : resources)
        resource->update();
}

shared<Shader> Shader::create() {
    struct Constructible_Shader : public Shader {};
    auto s = make_shared<Constructible_Shader>();
    resources.push_back(s);
    return s;
}

void Shader::setupProgram() {
    program.vertex      = vertex.get();
    program.tessControl = tessControl.get();
    program.tessEval    = tessEval.get();
    program.geometry    = geometry.get();
    program.fragment    = fragment.get();

    program.create();
    program.link();
}

void Shader::update() {
    Thread::Render::runNextFrame([this] {
        auto needsReload = vertex.tryUpdate(program.vertex);
        needsReload = tessControl.tryUpdate(program.tessControl) || needsReload;
        needsReload = tessEval.tryUpdate(program.tessEval) || needsReload;
        needsReload = geometry.tryUpdate(program.geometry) || needsReload;
        needsReload = fragment.tryUpdate(program.fragment) || needsReload;

        if (needsReload) {
            program.refresh();
            program.link();
        }
    });
}