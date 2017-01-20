#pragma once

#include "gl_structs.h"

struct GLstate_t {
    struct dims { GLint x, y; GLsizei width, height; };
    struct comp { GLint rgb, alpha; };
};

#ifndef GL_BINDING
constexpr GLenum GL_BINDING = GL_ALWAYS; // not reserved for anything else so why not
                                         // for future: GL_DONT_CARE
#endif

/// <summary>
/// Defines an OpenGL rendering state.
/// </summary>
template<GLenum gle, GLenum... mod>
struct GLstate {
    /// <summary>
    /// Applies this state to OpenGL.
    /// </summary>
    void apply() const;

    /// <summary>
    /// Captures the current OpenGL state.
    /// </summary>
    void capture();
};

template<GLenum GLE>
struct GLstate<GL_ENABLE_BIT, GLE> {
public:
    GLstate() { capture(); }
    ~GLstate() { apply(); }

    void apply() const {
        if (enabled) { GL_CHECK(glEnable(GLE)); } else { GL_CHECK(glDisable(GLE)); }
    }

    void capture() {
        GL_CHECK(enabled = glIsEnabled(GLE));
    }

private:
    GLboolean enabled;
};

template<>
struct GLstate<GL_DEPTH, GL_DEPTH_WRITEMASK> {
public:
    GLstate() { capture(); }
    ~GLstate() { apply(); }

    void apply() const {
        GL_CHECK(glDepthMask(enabled));
    }

    void capture() {
        enabled = getGLVal(GL_DEPTH_WRITEMASK);
    }

private:
    GLboolean enabled;
};

template<>
struct GLstate<GL_DEPTH> {
public:
    GLstate() { capture(); }
    ~GLstate() { apply(); }

    void apply() const {
        if (enabled) { GL_CHECK(glEnable(GL_DEPTH_TEST)); } else { GL_CHECK(glDisable(GL_DEPTH_TEST)); }
        GL_CHECK(glDepthMask(mask));
        GL_CHECK(glDepthFunc(func));
    }

    void capture() {
        GL_CHECK(enabled = glIsEnabled(GL_DEPTH_TEST));
        GL_CHECK(mask = getGLVal(GL_DEPTH_WRITEMASK));
        GL_CHECK(func = getGLVal(GL_DEPTH_FUNC));
    }

private:
    GLboolean enabled;
    GLboolean mask;
    GLenum func;
};

template<>
struct GLstate<GL_BLEND> {
    GLstate() { capture(); }
    ~GLstate() { apply(); }

    void apply() const {
        if (enabled) { GL_CHECK(glEnable(GL_BLEND)); } else { GL_CHECK(glDisable(GL_BLEND)); }
        GL_CHECK(glBlendEquationSeparate(equation.rgb, equation.alpha));
        GL_CHECK(glBlendFuncSeparate(src.rgb, dst.rgb, src.alpha, dst.alpha));
    }

    void capture() {
        GL_CHECK(enabled = glIsEnabled(GL_BLEND));
        src.rgb = getGLVal(GL_BLEND_SRC_RGB);
        src.alpha = getGLVal(GL_BLEND_SRC_ALPHA);
        dst.rgb = getGLVal(GL_BLEND_DST_RGB);
        dst.alpha = getGLVal(GL_BLEND_DST_ALPHA);
        equation.rgb = getGLVal(GL_BLEND_EQUATION_RGB);
        equation.alpha = getGLVal(GL_BLEND_EQUATION_ALPHA);
    }

private:
    GLboolean enabled;
    GLstate_t::comp src;
    GLstate_t::comp dst;
    GLstate_t::comp equation;
};

template<>
struct GLstate<GL_CULL_FACE> {
    GLstate() { capture(); }
    ~GLstate() { apply(); }

    void apply() const {
        if (enabled) { GL_CHECK(glEnable(GL_CULL_FACE)); }
        else { GL_CHECK(glDisable(GL_CULL_FACE)); }
        GL_CHECK(glFrontFace(front));
    }

    void capture() {
        GL_CHECK(enabled = glIsEnabled(GL_CULL_FACE));
        front = getGLVal(GL_FRONT_FACE);
    }

private:
    GLboolean enabled;
    GLenum front;
};

template<>
struct GLstate<GL_VIEWPORT> {
    GLstate() { capture(); }
    ~GLstate() { apply(); }

    void apply() const {
        Window::viewport(viewport.x, viewport.y, viewport.width, viewport.height);
    }

    void capture() {
        GL_CHECK(glGetIntegerv(GL_VIEWPORT, &viewport.x));
    }

private:
    GLstate_t::dims viewport;
};

template<>
struct GLstate<GL_SCISSOR_BOX> {
    GLstate() { capture(); }
    ~GLstate() { apply(); }

    void apply() const {
        if (enabled) { GL_CHECK(glEnable(GL_SCISSOR_TEST)); }
        else { GL_CHECK(glDisable(GL_SCISSOR_TEST)); }
        GL_CHECK(glScissor(scissorBox.x, scissorBox.y, scissorBox.height, scissorBox.height));
    }

    void capture() {
        GL_CHECK(enabled = glIsEnabled(GL_SCISSOR_TEST));
        GL_CHECK(glGetIntegerv(GL_SCISSOR_BOX, &scissorBox.x));
    }

private:
    GLboolean enabled;
    GLstate_t::dims scissorBox;
};

template<>
struct GLstate<GL_BINDING> {
    GLstate() { capture(); }
    ~GLstate() { apply(); }

    void apply() const {
        GL_CHECK(glUseProgram(program));
        GL_CHECK(glActiveTexture(activeTexture));
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, texture));
        GL_CHECK(glBindVertexArray(vertexArray));
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, arrayBuffer));
        GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementArrayBuffer));
    }

    void capture() {
        program = getGLVal(GL_CURRENT_PROGRAM);
        texture = getGLVal(GL_TEXTURE_BINDING_2D);
        activeTexture = getGLVal(GL_ACTIVE_TEXTURE);
        arrayBuffer = getGLVal(GL_ARRAY_BUFFER_BINDING);
        elementArrayBuffer = getGLVal(GL_ELEMENT_ARRAY_BUFFER_BINDING);
        vertexArray = getGLVal(GL_VERTEX_ARRAY_BINDING);
    }

private:
    GLint program;
    GLint texture;
    GLint activeTexture;
    GLint arrayBuffer;
    GLint elementArrayBuffer;
    GLint vertexArray;
};

template<>
struct GLstate<GL_BINDING, GL_TEXTURE_BINDING_2D> {
    GLstate() { capture(); }
    ~GLstate() { apply(); }

    void apply() const {
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, bound));
    }

    void capture() {
        bound = getGLVal(GL_TEXTURE_BINDING_2D);
    }

private:
    GLint bound;
};

/// <summary>
/// Defines a scoped state helper.
/// </summary>
class GLsavestate {
    struct state {
        GLstate<GL_BINDING> bindings;
        GLstate<GL_BLEND> blend;
        GLstate<GL_DEPTH> depth;
        GLstate<GL_CULL_FACE> cull;
        GLstate<GL_VIEWPORT> viewport;
        GLstate<GL_SCISSOR_BOX> scissorBox;
    };
public:
    /// <summary>
    /// Creates a new state helper and pushes the current OpenGL state.
    /// </summary>
    GLsavestate() {
        push();
    }

    /// <summary>
    /// Destroys this state helper and pops the current OpenGL state.
    /// </summary>
    ~GLsavestate() {
        pop();
    }

    static bool empty();
    static state* peek();
    static void push();
    static bool pop();

private:
    static std::vector<state> states;
};