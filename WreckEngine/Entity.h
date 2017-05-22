#pragma once

#include "smart_ptr.h"

#include "Random.h"

#include "Renderable.h"
#include "Transform.h"

class Entity
{
public:
    Entity() = default;
    explicit Entity(shared<GraphicsWorker> s);
    Entity(vec3 p, vec3 sc, vec3 rA, float r, shared<GraphicsWorker> s);

    virtual ~Entity() = default;

    Transform transform;
    bool active = true;

    void* id = (void*)Random::get(); // meant to identify the object for debugging purposes

    virtual void update(double dt) { };
    virtual void physicsUpdate(double dt) {};
    virtual void draw();

    template<class T> bool isType() const { return dynamic_cast<T*>(this) != nullptr; }
protected:
    shared<GraphicsWorker> shape;
};

class TransformEntity : public Entity {
public:
    void draw() override {}
};

class LogicEntity : public Entity {
    using update_func = void(*)(LogicEntity*, double);
public:
    explicit LogicEntity(update_func f) : Entity(), custom_update(f) { }
    update_func custom_update;
    void update(double dt) { custom_update(this, dt); }
    void draw() {}
};