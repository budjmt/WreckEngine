#include "TextureRead.h"
#include "External.h"

namespace File
{
    static_assert(sizeof(GLint) == sizeof(GLenum), "GLint and GLenum sizes differ!!!");

    constexpr GLint getFormatPitch(GLenum format) {
        // NOTE - See Table 2 from http://docs.gl/gl4/glTexImage2D
        switch (format) {
        case GL_R8:
        case GL_R8_SNORM:
            return 1;
        case GL_R16:
        case GL_R16_SNORM:
        case GL_RG8:
        case GL_RG8_SNORM:
        case GL_RG16:
        case GL_RG16_SNORM:
            return 2;
        case GL_R3_G3_B2:
            return 1;
        case GL_RGB4:
        case GL_RGB5:
            return 2;
        case GL_RGB8:
        case GL_RGB8_SNORM:
            return 3;
        case GL_RGB10:
            return 4;
        case GL_RGB12:
            return 5;
        case GL_RGB16_SNORM:
            return 6;
        case GL_RGBA2:
            return 1;
        case GL_RGBA4:
        case GL_RGB5_A1:
            return 2;
        case GL_RGBA8:
        case GL_RGBA8_SNORM:
        case GL_RGB10_A2:
        case GL_RGB10_A2UI:
            return 4;
        case GL_RGBA12:
            return 6;
        case GL_RGBA16:
            return 8;
        case GL_SRGB8:
            return 3;
        case GL_SRGB8_ALPHA8:
            return 4;
        case GL_R16F:
            return 2;
        case GL_RG16F:
            return 4;
        case GL_RGB16F:
            return 6;
        case GL_RGBA16F:
            return 8;
        case GL_R32F:
            return 4;
        case GL_RG32F:
            return 8;
        case GL_RGB32F:
            return 12;
        case GL_RGBA32F:
            return 16;
        case GL_R11F_G11F_B10F:
        case GL_RGB9_E5:
            return 4;
        case GL_R8I:
        case GL_R8UI:
            return 1;
        case GL_R16I:
        case GL_R16UI:
            return 2;
        case GL_R32I:
        case GL_R32UI:
            return 4;
        case GL_RG8I:
        case GL_RG8UI:
            return 2;
        case GL_RG16I:
        case GL_RG16UI:
            return 4;
        case GL_RG32I:
        case GL_RG32UI:
            return 8;
        case GL_RGB8I:
        case GL_RGB8UI:
            return 3;
        case GL_RGB16I:
        case GL_RGB16UI:
            return 6;
        case GL_RGB32I:
        case GL_RGB32UI:
            return 12;
        case GL_RGBA8I:
        case GL_RGBA8UI:
            return 4;
        case GL_RGBA16I:
        case GL_RGBA16UI:
            return 8;
        case GL_RGBA32I:
        case GL_RGBA32UI:
            return 16;
        }
        return 0;
    }

    // NOTE - May still need this??
    constexpr GLenum getBaseFormat(GLenum internalFormat) {
        switch (internalFormat) {
        case GL_R8:             return GL_RED;
        case GL_R8_SNORM:       return GL_RED;
        case GL_R16:            return GL_RED;
        case GL_R16_SNORM:      return GL_RED;
        case GL_RG8:            return GL_RG;
        case GL_RG8_SNORM:      return GL_RG;
        case GL_RG16:           return GL_RG;
        case GL_RG16_SNORM:     return GL_RG;
        case GL_R3_G3_B2:       return GL_RGB;
        case GL_RGB4:           return GL_RGB;
        case GL_RGB5:           return GL_RGB;
        case GL_RGB8:           return GL_RGB;
        case GL_RGB8_SNORM:     return GL_RGB;
        case GL_RGB10:          return GL_RGB;
        case GL_RGB12:          return GL_RGB;
        case GL_RGB16_SNORM:    return GL_RGB;
        case GL_RGBA2:          return GL_RGB;
        case GL_RGBA4:          return GL_RGB;
        case GL_RGB5_A1:        return GL_RGBA;
        case GL_RGBA8:          return GL_RGBA;
        case GL_RGBA8_SNORM:    return GL_RGBA;
        case GL_RGB10_A2:       return GL_RGBA;
        case GL_RGB10_A2UI:     return GL_RGBA;
        case GL_RGBA12:         return GL_RGBA;
        case GL_RGBA16:         return GL_RGBA;
        case GL_SRGB8:          return GL_RGB;
        case GL_SRGB8_ALPHA8:   return GL_RGBA;
        case GL_R16F:           return GL_RED;
        case GL_RG16F:          return GL_RG;
        case GL_RGB16F:         return GL_RGB;
        case GL_RGBA16F:        return GL_RGBA;
        case GL_R32F:           return GL_RED;
        case GL_RG32F:          return GL_RG;
        case GL_RGB32F:         return GL_RGB;
        case GL_RGBA32F:        return GL_RGBA;
        case GL_R11F_G11F_B10F: return GL_RGB;
        case GL_RGB9_E5:        return GL_RGB;
        case GL_R8I:            return GL_RED;
        case GL_R8UI:           return GL_RED;
        case GL_R16I:           return GL_RED;
        case GL_R16UI:          return GL_RED;
        case GL_R32I:           return GL_RED;
        case GL_R32UI:          return GL_RED;
        case GL_RG8I:           return GL_RG;
        case GL_RG8UI:          return GL_RG;
        case GL_RG16I:          return GL_RG;
        case GL_RG16UI:         return GL_RG;
        case GL_RG32I:          return GL_RG;
        case GL_RG32UI:         return GL_RG;
        case GL_RGB8I:          return GL_RGB;
        case GL_RGB8UI:         return GL_RGB;
        case GL_RGB16I:         return GL_RGB;
        case GL_RGB16UI:        return GL_RGB;
        case GL_RGB32I:         return GL_RGB;
        case GL_RGB32UI:        return GL_RGB;
        case GL_RGBA8I:         return GL_RGBA;
        case GL_RGBA8UI:        return GL_RGBA;
        case GL_RGBA16I:        return GL_RGBA;
        case GL_RGBA16UI:       return GL_RGBA;
        case GL_RGBA32I:        return GL_RGBA;
        case GL_RGBA32UI:       return GL_RGBA;
        }
        return GL_NONE;
    }

    void readTexture(GLtexture tex, GLenum target, GLenum format, GLenum type, ReadTextureCallback callback) {
        Thread::Render::runNextFrame([=]() {
            tex.bind();

            GLint width, height;
            GLenum format;
            glGetTexParameteriv(target, GL_TEXTURE_WIDTH, &width);
            glGetTexParameteriv(target, GL_TEXTURE_HEIGHT, &height);
            glGetTexParameteriv(target, GL_TEXTURE_INTERNAL_FORMAT, reinterpret_cast<GLint*>(&format));

            GLint pitch = getFormatPitch(format);
            unique<unsigned char[]> pixels = make_unique<unsigned char[]>(height * width * pitch);

            glGetTexImage(target, 0, format, type, pixels.get());

            if (callback) callback(std::move(pixels));
        });
    }
}
