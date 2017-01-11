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

// special method for updates that must occur after all others; these cannot be implemented by entities
void Game::postUpdate() {
#if DEBUG
if(drawDebug)
    DrawDebug::getInstance().update();
#endif
}

void Game::draw() {
    currState->draw();
#if DEBUG
    if (drawDebug)
        DrawDebug::getInstance().draw(&renderer.deferred.objects, &renderer.forward.objects);
#endif
    renderer.render();
}