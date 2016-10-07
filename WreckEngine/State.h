#pragma once

#include <vector>

#include "Entity.h"
#include "Event.h"

//-----------------------------------------
// States aren't too complex, but very powerful. They handle two things:
//    - Entities
//    - Events
// It just handles entity updates and has an event handler. Pretty simple.
//-----------------------------------------
class State {
public:
	State(const std::string _name) 
		: name(_name), handler(EventHandler(this, EventHandler::add(name + "_state"), nullptr)) {}

	std::function<void(Event)>& handler_func = handler.handler;

	void addEntity(shared<Entity> e);

	void update(double dt);
	void draw();
private:
	const std::string name;
	std::vector<shared<Entity>> entities;
	EventHandler handler;
};