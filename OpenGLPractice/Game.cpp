#include "Game.h"
#include <iostream>

Game::Game() 
{
	if (DEBUG)
		debug = new DrawDebug();
}

Game::Game(GLuint prog)
{
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
	if (DEBUG)
		delete debug;
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
	for (Entity* e : entities) {
		if (e->active)
			e->draw();
	}
	if(DEBUG)
		debug->draw();
}