#pragma once

#include "Game.h"

class TessellatorTest : public Game {
public:
    TessellatorTest();
    void update(double dt) override;
    void postUpdate() override;
    void draw() override;
};