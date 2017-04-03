#pragma once

#include "gl_structs.h"

namespace File
{
    using ReadTextureCallback = std::function<void(unique<unsigned char[]>)>;

    void readTexture(GLtexture tex, GLenum target, GLenum format, GLenum type, ReadTextureCallback callback);

    inline void readTexture(GLtexture tex, GLenum target, ReadTextureCallback callback) {
        readTexture(tex, target, GL_RGBA, GL_UNSIGNED_BYTE, callback);
    }
}
