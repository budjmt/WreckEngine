#include "ComputeEntity.h"

void ComputeEntity::update(double dt)
{
    timeSinceUpdate += static_cast<float>(Time::delta);
}

void ComputeEntity::draw()
{
    if (timeSinceUpdate >= updateFreq)
    {
        program.use();

        if (update_uniforms)
            update_uniforms(program);

        program.dispatch(dispatchSize.x, dispatchSize.y, dispatchSize.z);

        if (synchronize)
            GLsynchro::barrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        timeSinceUpdate -= updateFreq;
    }
}

void ComputeTextureEntity::draw()
{
    GLboolean layered = GL_FALSE;
    if (texture.target == GL_TEXTURE_CUBE_MAP ||
        texture.target == GL_TEXTURE_CUBE_MAP_ARRAY)
    {
        layered = GL_TRUE;
    }

    texture.bindImage(access, format, 0, 0, layered);
    ComputeEntity::draw();
}
