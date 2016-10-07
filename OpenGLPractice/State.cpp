#include "State.h"

void State::addEntity(shared<Entity> e) { entities.push_back(e); }

void State::update(double dt) {
	for (auto e : entities) {
		if (e->active)
			e->update(dt);
	}
}

void State::draw() {
	for (auto e : entities) {
		if (e->active)
			e->draw();
	}
}