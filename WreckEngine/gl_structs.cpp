#include "gl_structs.h"
#include "GLError.h"

GLint GLtexture::MAX_TEXTURES = -1;
GLint GLframebuffer::MAX_COLOR_ATTACHMENTS = -1;

GLint GLframebuffer::boundFBO = 0;

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
    GL_CHECK(glUseProgram(bound.program));
    GL_CHECK(glActiveTexture(bound.activeTexture));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, bound.texture));
    GL_CHECK(glBindVertexArray(bound.vertexArray));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, bound.arrayBuffer));
    GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bound.elementArrayBuffer));
    GL_CHECK(glBlendEquationSeparate(blend.equation.rgb, blend.equation.alpha));
    GL_CHECK(glBlendFunc(blend.src, blend.dst));
    if (enable.blend)       { GL_CHECK(glEnable(GL_BLEND));        } else { GL_CHECK(glDisable(GL_BLEND)); };
    if (enable.cullFace)    { GL_CHECK(glEnable(GL_CULL_FACE));    } else { GL_CHECK(glDisable(GL_CULL_FACE)); };
    if (enable.depthTest)   { GL_CHECK(glEnable(GL_DEPTH_TEST));   } else { GL_CHECK(glDisable(GL_DEPTH_TEST)); };
    if (enable.scissorTest) { GL_CHECK(glEnable(GL_SCISSOR_TEST)); } else { GL_CHECK(glDisable(GL_SCISSOR_TEST)); };
    GL_CHECK(glViewport(viewport.x, viewport.y, viewport.width, viewport.height));
    GL_CHECK(glScissor(scissorBox.x, scissorBox.y, scissorBox.height, scissorBox.height));
}

/// <summary>
/// Captures the current OpenGL state.
/// </summary>
void GLstate::capture()
{
    GL_CHECK(glGetIntegerv(GL_CURRENT_PROGRAM, &bound.program));
    GL_CHECK(glGetIntegerv(GL_TEXTURE_BINDING_2D, &bound.texture));
    GL_CHECK(glGetIntegerv(GL_ACTIVE_TEXTURE, &bound.activeTexture));
    GL_CHECK(glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &bound.arrayBuffer));
    GL_CHECK(glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &bound.elementArrayBuffer));
    GL_CHECK(glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &bound.vertexArray));
    GL_CHECK(glGetIntegerv(GL_BLEND_SRC, &blend.src));
    GL_CHECK(glGetIntegerv(GL_BLEND_DST, &blend.dst));
    GL_CHECK(glGetIntegerv(GL_BLEND_EQUATION_RGB, &blend.equation.rgb));
    GL_CHECK(glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &blend.equation.alpha));
    GL_CHECK(glGetIntegerv(GL_VIEWPORT, &viewport.x));
    GL_CHECK(glGetIntegerv(GL_SCISSOR_BOX, &scissorBox.x));

    GL_CHECK(enable.blend       = glIsEnabled(GL_BLEND));
    GL_CHECK(enable.cullFace    = glIsEnabled(GL_CULL_FACE));
    GL_CHECK(enable.depthTest   = glIsEnabled(GL_DEPTH_TEST));
    GL_CHECK(enable.scissorTest = glIsEnabled(GL_SCISSOR_TEST));
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
