#include "Game.h"
#include <iostream>

Game::Game(GLprogram prog) : shader(prog) { }

void Game::addState(shared<State> s) { 
	if (!states.size()) 
		currState = s.get(); 
	states.push_back(s); 
}

void Game::update(double dt) {
	currState->update(dt);
}

void Game::draw() {
	shader.use();
	currState->draw();
#if DEBUG
		DrawDebug::getInstance().draw();
		shader.use();
#endif
}