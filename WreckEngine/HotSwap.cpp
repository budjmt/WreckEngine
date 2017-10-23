#include "HotSwap.h"

using namespace HotSwap;

std::vector<shared<SwapResource>> HotSwap::resources;

void HotSwap::main() {
    for (auto& resource : resources)
        resource->update();
}

shared<Shader> Shader::create(std::function<void()> callback) {
    struct Constructible_Shader : public Shader { Constructible_Shader(std::function<void()> c) : Shader(c) {}; };
    auto s = make_shared<Constructible_Shader>(callback);
    resources.push_back(s);
    return s;
}

GLprogram Shader::setupProgram() {
    _program.vertex      = vertex.get();
    _program.tessControl = tessControl.get();
    _program.tessEval    = tessEval.get();
    _program.geometry    = geometry.get();
    _program.fragment    = fragment.get();
    _program.compute     = compute.get();

    _program.create();
    _program.link();
    return _program;
}

void Shader::update() {
    std::vector<std::pair<Resource<File::Extension::GLSL>&, GLshader&>> updates;
    if (vertex.checkForUpdate())      updates.push_back({ vertex,      _program.vertex });
    if (tessControl.checkForUpdate()) updates.push_back({ tessControl, _program.tessControl });
    if (tessEval.checkForUpdate())    updates.push_back({ tessEval,    _program.tessEval });
    if (geometry.checkForUpdate())    updates.push_back({ geometry,    _program.geometry });
    if (fragment.checkForUpdate())    updates.push_back({ fragment,    _program.fragment });
    if (compute.checkForUpdate())     updates.push_back({ compute,     _program.compute });

    Thread::Render::runNextFrame([this, updates = std::move(updates)] {
        bool needsRelink = false;
        for (auto& [resource, shader] : updates) {
            needsRelink = resource.getUpdate(shader) || needsRelink;
        }
        if (needsRelink) {
            _program.refresh();
            _program.link();
            if (callback) callback();
        }
    });
}