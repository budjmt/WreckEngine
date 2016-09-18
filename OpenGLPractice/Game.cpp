#include "Game.h"
#include <iostream>

Game::Game() { }
Game::Game(GLprogram prog) : shader(prog) { }

Entity* Game::operator[](size_t index) { return entities[index].get(); }

void Game::update(double dt) {
	for (auto e : entities) {
		if (e->active)
			e->update(dt);
	}
}

void Game::draw() {
	shader.use();
	for (auto e : entities) {
		if (e->active)
			e->draw();
	}
#if DEBUG
		DrawDebug::getInstance().draw();
		shader.use();
#endif
}