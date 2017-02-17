#include "gl_structs.h"
#include "GLstate.h"
#include "GLError.h"

GLint GLtexture::MAX_TEXTURES = -1;
GLint GLframebuffer::MAX_COLOR_ATTACHMENTS = -1;
GLprogram::uivec3 GLprogram::MAX_COMPUTE_WORK_GROUPS {};

GLint GLframebuffer::boundFBO = 0;

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
