#pragma once

#include "Render.h"
#include "Transform.h"

// Anything that interacts with the GPU is a graphics worker; they should only do work on the render thread
class GraphicsWorker {
public:
    Render::Info material;
    // updates/binds material data at draw-time; intended for dispatch
    // pass an Entity pointer if applicable, otherwise nullptr
    virtual void draw(Transform* t, Entity* entity);
};