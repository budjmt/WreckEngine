#include "Game.h"
#include <iostream>

Game::Game()
    : currState(nullptr)
    , drawDebug(true)
{
}

Game::Game(GLprogram prog)
    : currState(nullptr)
    , shader(prog)
    , drawDebug(true)
{
}

void Game::addState(shared<State> s) { 
    if (!states.size()) 
        currState = s.get(); 
    states.push_back(s);
}

void Game::update(double dt) {
    if (!currState) return;
    currState->update(dt);
}

void Game::draw() {
    if (shader()) shader.use();
    if (currState) currState->draw();
#if DEBUG
    if (drawDebug)
    {
        DrawDebug::getInstance().draw();
        shader.use();
    }
#endif
}