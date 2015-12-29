#include "Game.h"
#include <iostream>

Game::Game() 
{
}

Game::Game(GLuint prog)
{
	shader = prog;
}

Game::Game(const Game& other) 
{
	for(Entity* e : other.entities)
		entities.push_back(e);
	for(Drawable* s : other.shapes)
		shapes.push_back(s);
}

Game::~Game(void)
{
	for(Entity* e : entities)
		delete e;
	for(Drawable* s : shapes)
		delete s;
}

Entity* Game::operator[](int index) {
	return entities[index];
}

void Game::update(double dt) {
	for (Entity* e : entities) {
		if (e->active)
			e->update(dt);
	}
}

void Game::draw() {
	glUseProgram(shader);
	for (Entity* e : entities) {
		if (e->active)
			e->draw();
	}
#if DEBUG
		DrawDebug::getInstance().draw();
		glUseProgram(shader);
#endif
}