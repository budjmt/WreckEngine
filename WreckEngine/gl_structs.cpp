#include "gl_structs.h"
#include "GLError.h"

#pragma region GLstate Definition

std::vector<GLstate> GLstate::s_States;

/**
 * \brief Creates a new OpenGL state.
 */
GLstate::GLstate()
{
    capture();
}

/**
 * \brief Applies this state to OpenGL.
 */
void GLstate::apply()
{
    GL_CHECK(glUseProgram(m_Program));
    GL_CHECK(glActiveTexture(m_ActiveTexture));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, m_Texture));
    GL_CHECK(glBindVertexArray(m_VertexArray));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, m_ArrayBuffer));
    GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ElementArrayBuffer));
    GL_CHECK(glBlendEquationSeparate(m_BlendEquationRGB, m_BlendEquationAlpha));
    GL_CHECK(glBlendFunc(m_BlendSrc, m_BlendDst));
    if (m_EnableBlend)       { GL_CHECK(glEnable(GL_BLEND));        } else { GL_CHECK(glDisable(GL_BLEND)); };
    if (m_EnableCullFace)    { GL_CHECK(glEnable(GL_CULL_FACE));    } else { GL_CHECK(glDisable(GL_CULL_FACE)); };
    if (m_EnableDepthTest)   { GL_CHECK(glEnable(GL_DEPTH_TEST));   } else { GL_CHECK(glDisable(GL_DEPTH_TEST)); };
    if (m_EnableScissorTest) { GL_CHECK(glEnable(GL_SCISSOR_TEST)); } else { GL_CHECK(glDisable(GL_SCISSOR_TEST)); };
    GL_CHECK(glViewport(m_Viewport[0], m_Viewport[1], (GLsizei)m_Viewport[2], (GLsizei)m_Viewport[3]));
    GL_CHECK(glScissor(m_ScissorBox[0], m_ScissorBox[1], (GLsizei)m_ScissorBox[2], (GLsizei)m_ScissorBox[3]));
}

/**
 * \brief Captures the current OpenGL state.
 */
void GLstate::capture()
{
    GL_CHECK(glGetIntegerv(GL_CURRENT_PROGRAM, &m_Program));
    GL_CHECK(glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_Texture));
    GL_CHECK(glGetIntegerv(GL_ACTIVE_TEXTURE, &m_ActiveTexture));
    GL_CHECK(glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &m_ArrayBuffer));
    GL_CHECK(glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &m_ElementArrayBuffer));
    GL_CHECK(glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &m_VertexArray));
    GL_CHECK(glGetIntegerv(GL_BLEND_SRC, &m_BlendSrc));
    GL_CHECK(glGetIntegerv(GL_BLEND_DST, &m_BlendDst));
    GL_CHECK(glGetIntegerv(GL_BLEND_EQUATION_RGB, &m_BlendEquationRGB));
    GL_CHECK(glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &m_BlendEquationAlpha));
    GL_CHECK(glGetIntegerv(GL_VIEWPORT, m_Viewport));
    GL_CHECK(glGetIntegerv(GL_SCISSOR_BOX, m_ScissorBox));

    GL_CHECK(m_EnableBlend = glIsEnabled(GL_BLEND));
    GL_CHECK(m_EnableCullFace = glIsEnabled(GL_CULL_FACE));
    GL_CHECK(m_EnableDepthTest = glIsEnabled(GL_DEPTH_TEST));
    GL_CHECK(m_EnableScissorTest = glIsEnabled(GL_SCISSOR_TEST));
}

/**
 * \brief Gets the OpenGL state at the top of the stack.
 *
 * \return The OpenGL state at the top of the stack, or null if there are no states.
 */
GLstate* GLstate::peek()
{
    if (s_States.size())
    {
        return &s_States.back();
    }
    return nullptr;
}

/**
 * \brief Pushes the current OpenGL state.
 */
void GLstate::push()
{
    s_States.push_back(GLstate());
}

/**
 * \brief Attempts to pop an OpenGL state off of the stack.
 *
 * \return True if there was a state to pop, otherwise false.
 */
bool GLstate::pop()
{
    if (s_States.size())
    {
        s_States.back().apply();
        s_States.pop_back();
        return true;
    }
    return false;
}

#pragma endregion
