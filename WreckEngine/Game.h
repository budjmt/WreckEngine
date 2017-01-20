#pragma once

#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include <vector>
#include <cmath>

#include "smart_ptr.h"

#include "State.h"
#include "RenderSpecial.h"
#include "DrawDebug.h"

class Game
{
public:
    Game(const size_t gBufferSize) : renderer(Render::LitRenderer(gBufferSize)) {}
    virtual ~Game() = default;

    void addState(shared<State> s);

    virtual void update(double dt);
    virtual void postUpdate();
    virtual void physicsUpdate(double dt);
    virtual void draw();
protected:
    Render::LitRenderer renderer;

    std::vector<shared<State>> states;
    State* currState = nullptr;
    bool drawDebug = true;
};

