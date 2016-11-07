#include "gl_structs.h"

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
    glUseProgram(m_Program);
    glActiveTexture(m_ActiveTexture);
    glBindTexture(GL_TEXTURE_2D, m_Texture);
    glBindVertexArray(m_VertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, m_ArrayBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ElementArrayBuffer);
    glBlendEquationSeparate(m_BlendEquationRGB, m_BlendEquationAlpha);
    glBlendFunc(m_BlendSrc, m_BlendDst);
    if (m_EnableBlend)       glEnable(GL_BLEND);        else glDisable(GL_BLEND);
    if (m_EnableCullFace)    glEnable(GL_CULL_FACE);    else glDisable(GL_CULL_FACE);
    if (m_EnableDepthTest)   glEnable(GL_DEPTH_TEST);   else glDisable(GL_DEPTH_TEST);
    if (m_EnableScissorTest) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
    glViewport(m_Viewport[0], m_Viewport[1], (GLsizei)m_Viewport[2], (GLsizei)m_Viewport[3]);
    glScissor(m_ScissorBox[0], m_ScissorBox[1], (GLsizei)m_ScissorBox[2], (GLsizei)m_ScissorBox[3]);
}

/**
 * \brief Captures the current OpenGL state.
 */
void GLstate::capture()
{
    glGetIntegerv(GL_CURRENT_PROGRAM, &m_Program);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_Texture);
    glGetIntegerv(GL_ACTIVE_TEXTURE, &m_ActiveTexture);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &m_ArrayBuffer);
    glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &m_ElementArrayBuffer);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &m_VertexArray);
    glGetIntegerv(GL_BLEND_SRC, &m_BlendSrc);
    glGetIntegerv(GL_BLEND_DST, &m_BlendDst);
    glGetIntegerv(GL_BLEND_EQUATION_RGB, &m_BlendEquationRGB);
    glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &m_BlendEquationAlpha);
    glGetIntegerv(GL_VIEWPORT, m_Viewport);
    glGetIntegerv(GL_SCISSOR_BOX, m_ScissorBox);

    m_EnableBlend = glIsEnabled(GL_BLEND);
    m_EnableCullFace = glIsEnabled(GL_CULL_FACE);
    m_EnableDepthTest = glIsEnabled(GL_DEPTH_TEST);
    m_EnableScissorTest = glIsEnabled(GL_SCISSOR_TEST);
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
