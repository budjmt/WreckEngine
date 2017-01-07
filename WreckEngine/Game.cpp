#include "Game.h"
#include <iostream>

void Game::addState(shared<State> s) { 
    if (!states.size()) 
        currState = s.get(); 
    states.push_back(s);
}

void Game::update(double dt) {
    currState->update(dt);
}

void Game::draw() {
    currState->draw();
#if DEBUG
    if (drawDebug)
        DrawDebug::getInstance().draw(&renderer.deferred.objects, &renderer.forward.objects);
#endif
    renderer.render();
}