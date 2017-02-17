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
    program.compute     = compute.get();

    program.create();
    program.link();
}

void Shader::update() {
    std::vector<std::pair<Resource<File::Extension::GLSL>&, GLshader&>> updates;
    if (vertex.checkForUpdate())      updates.push_back({ vertex, program.vertex });
    if (tessControl.checkForUpdate()) updates.push_back({ tessControl, program.tessControl });
    if (tessEval.checkForUpdate())    updates.push_back({ tessEval, program.tessEval });
    if (geometry.checkForUpdate())    updates.push_back({ geometry, program.geometry });
    if (fragment.checkForUpdate())    updates.push_back({ fragment, program.fragment });
    if (compute.checkForUpdate())     updates.push_back({ compute, program.compute });

    Thread::Render::runNextFrame([this, updates] {
        bool needsRelink = false;
        for (auto& u : updates) {
            needsRelink = u.first.getUpdate(u.second) || needsRelink;
        }
        if (needsRelink) {
            program.refresh();
            program.link();
        }
    });
}