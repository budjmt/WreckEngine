#include "gl_structs.h"
#include "GLError.h"

#pragma region GLstate Definition

std::vector<GLstate> GLstate::states;

/// <summary>
/// Creates a new OpenGL state.
/// </summary>
GLstate::GLstate()
{
    capture();
}

/// <summary>
/// Applies this state to OpenGL.
/// </summary>
void GLstate::apply()
{
    GL_CHECK(glUseProgram(boundProgram));
    GL_CHECK(glActiveTexture(activeTexture));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, boundTexture));
    GL_CHECK(glBindVertexArray(vertexArray));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, arrayBuffer));
    GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementArrayBuffer));
    GL_CHECK(glBlendEquationSeparate(blendEquationRGB, blendEquationAlpha));
    GL_CHECK(glBlendFunc(blendSrc, blendDst));
    if (enableBlend)       { GL_CHECK(glEnable(GL_BLEND));        } else { GL_CHECK(glDisable(GL_BLEND)); };
    if (enableCullFace)    { GL_CHECK(glEnable(GL_CULL_FACE));    } else { GL_CHECK(glDisable(GL_CULL_FACE)); };
    if (enableDepthTest)   { GL_CHECK(glEnable(GL_DEPTH_TEST));   } else { GL_CHECK(glDisable(GL_DEPTH_TEST)); };
    if (enableScissorTest) { GL_CHECK(glEnable(GL_SCISSOR_TEST)); } else { GL_CHECK(glDisable(GL_SCISSOR_TEST)); };
    GL_CHECK(glViewport(viewport[0], viewport[1], (GLsizei)viewport[2], (GLsizei)viewport[3]));
    GL_CHECK(glScissor(scissorBox[0], scissorBox[1], (GLsizei)scissorBox[2], (GLsizei)scissorBox[3]));
}

/// <summary>
/// Captures the current OpenGL state.
/// </summary>
void GLstate::capture()
{
    GL_CHECK(glGetIntegerv(GL_CURRENT_PROGRAM, &boundProgram));
    GL_CHECK(glGetIntegerv(GL_TEXTURE_BINDING_2D, &boundTexture));
    GL_CHECK(glGetIntegerv(GL_ACTIVE_TEXTURE, &activeTexture));
    GL_CHECK(glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &arrayBuffer));
    GL_CHECK(glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &elementArrayBuffer));
    GL_CHECK(glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vertexArray));
    GL_CHECK(glGetIntegerv(GL_BLEND_SRC, &blendSrc));
    GL_CHECK(glGetIntegerv(GL_BLEND_DST, &blendDst));
    GL_CHECK(glGetIntegerv(GL_BLEND_EQUATION_RGB, &blendEquationRGB));
    GL_CHECK(glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &blendEquationAlpha));
    GL_CHECK(glGetIntegerv(GL_VIEWPORT, viewport));
    GL_CHECK(glGetIntegerv(GL_SCISSOR_BOX, scissorBox));

    GL_CHECK(enableBlend       = glIsEnabled(GL_BLEND));
    GL_CHECK(enableCullFace    = glIsEnabled(GL_CULL_FACE));
    GL_CHECK(enableDepthTest   = glIsEnabled(GL_DEPTH_TEST));
    GL_CHECK(enableScissorTest = glIsEnabled(GL_SCISSOR_TEST));
}

/// <summary>
/// Checks to see if there are any cached states.
/// </summary>
/// <returns>True if there is a state on the stack, otherwise false.</returns>
bool GLstate::empty()
{
    return states.size() > 0;
}

/// <summary>
/// Gets the OpenGL state at the top of the stack.
/// </summary>
/// <returns>The OpenGL state at the top of the stack.</returns>
GLstate* GLstate::peek()
{
    return &states.back();
}

/// <summary>
/// Pushes the current OpenGL state.
/// </summary>
void GLstate::push()
{
    states.push_back(GLstate());
}

/// <summary>
/// Attempts to pop an OpenGL state off of the stack.
/// </summary>
/// <returns>True if there was a state to pop, otherwise false.</returns>
bool GLstate::pop()
{
    if (states.size())
    {
        states.back().apply();
        states.pop_back();
        return true;
    }
    return false;
}

#pragma endregion
