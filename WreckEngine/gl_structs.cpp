#include "gl_structs.h"
#include "GLstate.h"
#include "GLError.h"

GLint GLtexture::MAX_TEXTURES = -1;
GLint GLframebuffer::MAX_COLOR_ATTACHMENTS = -1;
GLprogram::uivec3 GLprogram::MAX_COMPUTE_WORK_GROUPS {};

GLint GLframebuffer::boundFBO = 0;

GLint GLtexture::getFormatPitch(GLenum format) {
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

#pragma region GLstate Definition

std::vector<GLsavestate::state> GLsavestate::states;

/// <summary>
/// Checks to see if there are any cached states.
/// </summary>
/// <returns>True if there is a state on the stack, otherwise false.</returns>
bool GLsavestate::empty()
{
    return states.size() > 0;
}

/// <summary>
/// Gets the OpenGL state at the top of the stack.
/// </summary>
/// <returns>The OpenGL state at the top of the stack.</returns>
GLsavestate::state* GLsavestate::peek()
{
    return &states.back();
}

/// <summary>
/// Pushes the current OpenGL state.
/// </summary>
void GLsavestate::push()
{
    states.push_back(state());
}

/// <summary>
/// Attempts to pop an OpenGL state off of the stack.
/// </summary>
/// <returns>True if there was a state to pop, otherwise false.</returns>
bool GLsavestate::pop()
{
    if (states.size())
    {
        states.pop_back();
        return true;
    }
    return false;
}

#pragma endregion
