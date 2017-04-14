#include "ComputeEntity.h"

void ComputeEntity::update(double dt)
{
    timeSinceUpdate += static_cast<float>(Time::delta);
}

void ComputeEntity::draw()
{
    if (timeSinceUpdate >= updateFreq)
    {
        Entity::draw();
        shape->material.shaders->program.dispatch(dispatchSize.x, dispatchSize.y, dispatchSize.z);

        if (synchronize)
            GLsynchro::barrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        timeSinceUpdate -= updateFreq;
    }
}

void ComputeTextureEntity::draw()
{
    GLint index = 0;
    for (auto& image : images) {
        image.tex.bindImage(image.access, image.format, index, 0, image.layered());
        ++index;
    }

    ComputeEntity::draw();
}
