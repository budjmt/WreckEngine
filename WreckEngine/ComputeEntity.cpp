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
    const GLboolean layered = texture.target == GL_TEXTURE_CUBE_MAP || texture.target == GL_TEXTURE_CUBE_MAP_ARRAY;

    texture.bindImage(access, format, index, 0, layered);
    ComputeEntity::draw();
}
