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
    glCheck(glUseProgram(m_Program));
    glCheck(glActiveTexture(m_ActiveTexture));
    glCheck(glBindTexture(GL_TEXTURE_2D, m_Texture));
    glCheck(glBindVertexArray(m_VertexArray));
    glCheck(glBindBuffer(GL_ARRAY_BUFFER, m_ArrayBuffer));
    glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ElementArrayBuffer));
    glCheck(glBlendEquationSeparate(m_BlendEquationRGB, m_BlendEquationAlpha));
    glCheck(glBlendFunc(m_BlendSrc, m_BlendDst));
    if (m_EnableBlend)       { glCheck(glEnable(GL_BLEND));        } else { glCheck(glDisable(GL_BLEND)); };
    if (m_EnableCullFace)    { glCheck(glEnable(GL_CULL_FACE));    } else { glCheck(glDisable(GL_CULL_FACE)); };
    if (m_EnableDepthTest)   { glCheck(glEnable(GL_DEPTH_TEST));   } else { glCheck(glDisable(GL_DEPTH_TEST)); };
    if (m_EnableScissorTest) { glCheck(glEnable(GL_SCISSOR_TEST)); } else { glCheck(glDisable(GL_SCISSOR_TEST)); };
    glCheck(glViewport(m_Viewport[0], m_Viewport[1], (GLsizei)m_Viewport[2], (GLsizei)m_Viewport[3]));
    glCheck(glScissor(m_ScissorBox[0], m_ScissorBox[1], (GLsizei)m_ScissorBox[2], (GLsizei)m_ScissorBox[3]));
}

/**
 * \brief Captures the current OpenGL state.
 */
void GLstate::capture()
{
    glCheck(glGetIntegerv(GL_CURRENT_PROGRAM, &m_Program));
    glCheck(glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_Texture));
    glCheck(glGetIntegerv(GL_ACTIVE_TEXTURE, &m_ActiveTexture));
    glCheck(glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &m_ArrayBuffer));
    glCheck(glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &m_ElementArrayBuffer));
    glCheck(glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &m_VertexArray));
    glCheck(glGetIntegerv(GL_BLEND_SRC, &m_BlendSrc));
    glCheck(glGetIntegerv(GL_BLEND_DST, &m_BlendDst));
    glCheck(glGetIntegerv(GL_BLEND_EQUATION_RGB, &m_BlendEquationRGB));
    glCheck(glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &m_BlendEquationAlpha));
    glCheck(glGetIntegerv(GL_VIEWPORT, m_Viewport));
    glCheck(glGetIntegerv(GL_SCISSOR_BOX, m_ScissorBox));

    glCheck(m_EnableBlend = glIsEnabled(GL_BLEND));
    glCheck(m_EnableCullFace = glIsEnabled(GL_CULL_FACE));
    glCheck(m_EnableDepthTest = glIsEnabled(GL_DEPTH_TEST));
    glCheck(m_EnableScissorTest = glIsEnabled(GL_SCISSOR_TEST));
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
