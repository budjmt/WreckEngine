#pragma once

#include <vector>

#include "Entity.h"
#include "Event.h"

//-----------------------------------------
// States aren't too complex, but very powerful. They handle two things:
//    - Entities
//    - Events
// It just handles entity updates and has an event handler. 
// This simple combination allows for states to:
//   - load and unload entities as needed, on their own time-frame
//   - "freeze" entity states while not active
//   - act as a go-between for individual entities
//   - reduce the number of necessary handlers
//-----------------------------------------
class State {
public:
    State(const std::string _name) 
        : name(_name), handler(Event::Handler(this, Event::Handler::add(name + "_state"), nullptr)) {}

    Event::Handler::func_t& handler_func = handler.handler;

    void addEntity(shared<Entity> e);

    void update(double dt);
    void physicsUpdate(double dt);
    void draw();
private:
    const std::string name;
    std::vector<shared<Entity>> entities;
    Event::Handler handler;
};