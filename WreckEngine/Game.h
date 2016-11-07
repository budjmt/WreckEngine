#pragma once

#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include <vector>
#include <cmath>

#include "smart_ptr.h"

#include "State.h"
#include "DrawDebug.h"

class Game
{
public:
    Game();
    Game(GLprogram prog);
    virtual ~Game() = default;

    void addState(shared<State> s);

    virtual void update(double dt);
    virtual void draw();
protected:
    GLprogram shader;
    std::vector<shared<State>> states;
    State* currState;
    bool drawDebug;
};

