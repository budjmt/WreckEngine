#pragma once

inline GLint getGLVal(GLenum value) { GLint val; GL_CHECK(glGetIntegerv(value, &val)); return val; }

#ifndef GL_BLEND_FUNC
constexpr GLenum GL_BLEND_FUNC = GL_3D; // just going for a value that won't be used in context
#endif
#ifndef GL_BINDING
constexpr GLenum GL_BINDING = GL_ALWAYS; // not reserved for anything else so why not
                                         // for future: GL_DONT_CARE
#endif

struct GLstate_dims { GLint x, y; GLsizei width, height; };
struct GLstate_comp {
    GLstate_comp() = default;
    GLstate_comp(GLint r, GLint a) : rgb(r), alpha(a) {}
    GLstate_comp(GLint v) : rgb(v), alpha(v) {}

    GLstate_comp(const GLstate_comp&) = default;
    GLstate_comp(GLstate_comp&&) = default;
    GLstate_comp& operator=(const GLstate_comp&) = default;
    GLstate_comp& operator=(GLstate_comp&&) = default;
    
    GLint rgb, alpha; 
};

namespace detail {

    template<typename Callable, typename Tuple>
    void tuple_for_each(Callable c, Tuple& t) {
        tuple_for_each_impl(t, c, std::make_index_sequence<std::tuple_size_v<Tuple>>{});
    }

    template<typename Tuple, typename Callable, size_t... I>
    void tuple_for_each_impl(Tuple& t, Callable c, std::index_sequence<I...>) {
        auto l = { (c(std::get<I>(t)), 0)... };
    }

    // helper class for fetching/applying GL state
    template<GLenum gle, GLenum... mod>
    struct GLstatefrag {
        // applies this state to OpenGL
        void apply() const;

        // fetches the current OpenGL state.
        void fetch();
    };

    template<GLenum GLE>
    struct GLstatefrag<GLE, GL_ENABLE_BIT> {
        void apply() const { if (enabled) { GL_CHECK(glEnable(GLE)); } else { GL_CHECK(glDisable(GLE)); } }
        void fetch() { GL_CHECK(enabled = glIsEnabled(GLE) == GL_TRUE); }

        bool enabled{ false };
    };

    template<> struct GLstatefrag<GL_DEPTH_TEST, GL_DEPTH_WRITEMASK> {
    public:
        void apply() const { GL_CHECK(glDepthMask((GLboolean)enabled)); }
        void fetch() { enabled = getGLVal(GL_DEPTH_WRITEMASK) == GL_TRUE; }

        bool enabled{ true };
    };

    template<> struct GLstatefrag<GL_DEPTH_TEST, GL_DEPTH_FUNC> {
    public:
        void apply() const { GL_CHECK(glDepthFunc(func)); }
        void fetch() { func = getGLVal(GL_DEPTH_FUNC); }

        GLint func{ GL_LESS };
    };

    template<>
    struct GLstatefrag<GL_DEPTH_TEST> {
    public:
        template<GLenum... Args> auto& get() { return std::get<GLstatefrag<GL_DEPTH_TEST, Args...>>(depthStore); }
        void apply() const { detail::tuple_for_each([](auto& x) { x.apply(); }, depthStore); }
        void fetch() { detail::tuple_for_each([](auto& x) { x.fetch(); }, depthStore); }
    private:
        std::tuple<
            GLstatefrag<GL_DEPTH_TEST, GL_ENABLE_BIT>,
            GLstatefrag<GL_DEPTH_TEST, GL_DEPTH_WRITEMASK>,
            GLstatefrag<GL_DEPTH_TEST, GL_DEPTH_FUNC>
        > depthStore;
    };

    template<> struct GLstatefrag<GL_BLEND, GL_BLEND_FUNC> {
        void apply() const {
            GL_CHECK(glBlendFuncSeparate(src.rgb, dst.rgb, src.alpha, dst.alpha));
        }
        void fetch() {
            src.rgb = getGLVal(GL_BLEND_SRC_RGB);
            src.alpha = getGLVal(GL_BLEND_SRC_ALPHA);
            dst.rgb = getGLVal(GL_BLEND_DST_RGB);
            dst.alpha = getGLVal(GL_BLEND_DST_ALPHA);
        }

        GLstate_comp src{ GL_ONE }, dst{ GL_ZERO };
    };

    template<> struct GLstatefrag<GL_BLEND, GL_BLEND_EQUATION> {
        void apply() const {
            GL_CHECK(glBlendEquationSeparate(val.rgb, val.alpha));
        }
        void fetch() {
            val.rgb = getGLVal(GL_BLEND_EQUATION_RGB);
            val.alpha = getGLVal(GL_BLEND_EQUATION_ALPHA);
        }

        GLstate_comp val{ GL_FUNC_ADD };
    };

    template<>
    struct GLstatefrag<GL_BLEND> {
        template<GLenum... Args> auto& get() { return std::get<GLstatefrag<GL_BLEND, Args...>>(blendStore); }
        void apply() const { detail::tuple_for_each([](auto& x) { x.apply(); }, blendStore); }
        void fetch() { detail::tuple_for_each([](auto& x) { x.fetch(); }, blendStore); }
    private:
        std::tuple <
            GLstatefrag<GL_BLEND, GL_ENABLE_BIT>,
            GLstatefrag<GL_BLEND, GL_BLEND_FUNC>,
            GLstatefrag<GL_BLEND, GL_BLEND_EQUATION>
        > blendStore;
    };

    template<> struct GLstatefrag<GL_CULL_FACE, GL_CULL_FACE_MODE> {
        void apply() const { GL_CHECK(glCullFace(mode)); }
        void fetch() { mode = getGLVal(GL_CULL_FACE_MODE); }

        GLint mode{ GL_BACK };
    };

    template<> struct GLstatefrag<GL_CULL_FACE, GL_FRONT_FACE> {
        void apply() const { GL_CHECK(glFrontFace(val)); }
        void fetch() { val = getGLVal(GL_FRONT_FACE); }

        GLint val{ GL_CCW };
    };

    template<>
    struct GLstatefrag<GL_CULL_FACE> {
        template<GLenum... Args> auto& get() { return std::get<GLstatefrag<GL_CULL_FACE, Args...>>(cullStore); }
        void apply() const { detail::tuple_for_each([](auto& x) { x.apply(); }, cullStore); }
        void fetch() { detail::tuple_for_each([](auto& x) { x.fetch(); }, cullStore); }
    private:
        std::tuple<
            GLstatefrag<GL_CULL_FACE, GL_ENABLE_BIT>,
            GLstatefrag<GL_CULL_FACE, GL_CULL_FACE_MODE>,
            GLstatefrag<GL_CULL_FACE, GL_FRONT_FACE>
        > cullStore;
    };

    template<>
    struct GLstatefrag<GL_VIEWPORT> {
        void apply() const {
            GL_CHECK(glViewport(viewport.x, viewport.y, viewport.width, viewport.height));
        }
        void fetch() { GL_CHECK(glGetIntegerv(GL_VIEWPORT, &viewport.x)); }

        GLstate_dims viewport{ 0, 0, 0, 0 }; // these values are fetched at startup
    };

    template<> struct GLstatefrag<GL_SCISSOR_TEST, GL_SCISSOR_BOX> {
        void apply() const { GL_CHECK(glScissor(scissorBox.x, scissorBox.y, scissorBox.width, scissorBox.height)); }
        void fetch() { GL_CHECK(glGetIntegerv(GL_SCISSOR_BOX, &scissorBox.x)); }

        GLstate_dims scissorBox{ 0, 0, 0, 0 }; // these values are fetched at startup
    };

    template<>
    struct GLstatefrag<GL_SCISSOR_TEST> {
        template<GLenum... Args> auto& get() { return std::get<GLstatefrag<GL_SCISSOR_TEST, Args...>>(scissorStore); }
        void apply() const { detail::tuple_for_each([](auto& x) { x.apply(); }, scissorStore); }
        void fetch() { detail::tuple_for_each([](auto& x) { x.fetch(); }, scissorStore); }
    private:
        std::tuple<
            GLstatefrag<GL_SCISSOR_TEST, GL_ENABLE_BIT>,
            GLstatefrag<GL_SCISSOR_TEST, GL_SCISSOR_BOX>
        > scissorStore;
    };

    template<> struct GLstatefrag<GL_BINDING, GL_PROGRAM> {
        void apply() const { GL_CHECK(glUseProgram(current)); }
        void fetch() { current = getGLVal(GL_CURRENT_PROGRAM); }

        GLint current{ 0 };
    };

    template<> struct GLstatefrag<GL_BINDING, GL_ACTIVE_TEXTURE> {
        void apply() const { GL_CHECK(glActiveTexture(active)); }
        void fetch() { active = getGLVal(GL_ACTIVE_TEXTURE); }

        GLint active{ GL_TEXTURE0 };
    };

    template<> struct GLstatefrag<GL_BINDING, GL_TEXTURE_BINDING_1D> {
        void apply() const { GL_CHECK(glBindTexture(GL_TEXTURE_1D, bound)); }
        void fetch() { bound = getGLVal(GL_TEXTURE_BINDING_1D); }

        GLint bound{ 0 };
    };

    template<> struct GLstatefrag<GL_BINDING, GL_TEXTURE_BINDING_2D> {
        void apply() const { GL_CHECK(glBindTexture(GL_TEXTURE_2D, bound)); }
        void fetch() { bound = getGLVal(GL_TEXTURE_BINDING_2D); }

        GLint bound{ 0 };
    };

    template<> struct GLstatefrag<GL_BINDING, GL_TEXTURE_BINDING_3D> {
        void apply() const { GL_CHECK(glBindTexture(GL_TEXTURE_3D, bound)); }
        void fetch() { bound = getGLVal(GL_TEXTURE_BINDING_3D); }

        GLint bound{ 0 };
    };

    template<> struct GLstatefrag<GL_BINDING, GL_ARRAY_BUFFER> {
        void apply() const { GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, bound)); }
        void fetch() { bound = getGLVal(GL_ARRAY_BUFFER_BINDING); }

        GLint bound{ 0 };
    };

    template<> struct GLstatefrag<GL_BINDING, GL_ELEMENT_ARRAY_BUFFER> {
        void apply() const { GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bound)); }
        void fetch() { bound = getGLVal(GL_ELEMENT_ARRAY_BUFFER_BINDING); }

        GLint bound{ 0 };
    };

    template<> struct GLstatefrag<GL_BINDING, GL_VERTEX_ARRAY> {
        void apply() const { GL_CHECK(glBindVertexArray(bound)); }
        void fetch() { bound = getGLVal(GL_VERTEX_ARRAY_BINDING); }

        GLint bound{ 0 };
    };

    template<> struct GLstatefrag<GL_BINDING, GL_DRAW_FRAMEBUFFER> {
        void apply() const { GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, bound)); }
        void fetch() { bound = getGLVal(GL_DRAW_FRAMEBUFFER_BINDING); }

        GLint bound{ 0 };
    };

    template<> struct GLstatefrag<GL_BINDING, GL_READ_FRAMEBUFFER> {
        void apply() const { GL_CHECK(glBindFramebuffer(GL_READ_FRAMEBUFFER, bound)); }
        void fetch() { bound = getGLVal(GL_READ_FRAMEBUFFER_BINDING); }

        GLint bound{ 0 };
    };

    template<>
    struct GLstatefrag<GL_BINDING> {
        template<GLenum... Args> auto& get() { return std::get<GLstatefrag<GL_BINDING, Args...>>(bindingStore); }
        void apply() const { detail::tuple_for_each([](auto& x) { x.apply(); }, bindingStore); }
        void fetch() { detail::tuple_for_each([](auto& x) { x.fetch(); }, bindingStore); }
    private:
        std::tuple<
            GLstatefrag<GL_BINDING, GL_PROGRAM>,
            GLstatefrag<GL_BINDING, GL_ACTIVE_TEXTURE>,
            GLstatefrag<GL_BINDING, GL_TEXTURE_BINDING_1D>,
            GLstatefrag<GL_BINDING, GL_TEXTURE_BINDING_2D>,
            GLstatefrag<GL_BINDING, GL_TEXTURE_BINDING_3D>,
            GLstatefrag<GL_BINDING, GL_ARRAY_BUFFER>,
            GLstatefrag<GL_BINDING, GL_ELEMENT_ARRAY_BUFFER>,
            GLstatefrag<GL_BINDING, GL_VERTEX_ARRAY>,
            GLstatefrag<GL_BINDING, GL_DRAW_FRAMEBUFFER>,
            GLstatefrag<GL_BINDING, GL_READ_FRAMEBUFFER>
        > bindingStore;
    };

    template<> struct GLstatefrag<GL_POLYGON_MODE> {
        void apply() const { glPolygonMode(GL_FRONT_AND_BACK, mode); }
        void fetch() { GLint temp[2]; GL_CHECK(glGetIntegerv(GL_POLYGON_MODE, temp)); mode = temp[0]; }

        GLint mode{ GL_FILL };
    };

    template<> struct GLstatefrag<GL_COLOR_CLEAR_VALUE> {
        void apply() const { glClearColor(color.r, color.g, color.b, color.a); }
        void fetch() { GL_CHECK(glGetFloatv(GL_COLOR_CLEAR_VALUE, &color.r)); }

        vec4 color;
    };
}

// Reflects the global state for OpenGL
// will remain accurate if state changes go through the GLstate functions
class GLglobal {
    std::tuple<
        detail::GLstatefrag<GL_BINDING>,
        detail::GLstatefrag<GL_BLEND>,
        detail::GLstatefrag<GL_DEPTH_TEST>,
        detail::GLstatefrag<GL_CULL_FACE>,
        detail::GLstatefrag<GL_VIEWPORT>,
        detail::GLstatefrag<GL_SCISSOR_TEST>,
        detail::GLstatefrag<GL_POLYGON_MODE>,
        detail::GLstatefrag<GL_COLOR_CLEAR_VALUE>
    > stateStore;

    using state_store_t = decltype(stateStore);

    template<GLenum Base, GLenum... Args> struct get_helper
    { static auto& get(state_store_t& state) { return std::get<detail::GLstatefrag<Base>>(state).get<Args...>(); } };

    template<GLenum Base> struct get_helper<Base>
    { static auto& get(state_store_t& state) { return std::get<detail::GLstatefrag<Base>>(state); } };

    static GLglobal saved, instance;

    template<GLenum... Args> friend struct GLstate;
public:
    template<GLenum... Args> auto& get() { return get_helper<Args...>::get(stateStore); }

    void apply() const { detail::tuple_for_each([](auto& x) { x.apply(); }, stateStore); }
    void fetch() { detail::tuple_for_each([](auto& x) { x.fetch(); }, stateStore); }

    static void save() { saved = instance; }
    static void restore() { instance = saved; saved.apply(); }
};

// represents a fragment of GL state; can be manipulated to affect actual GL state
template<GLenum... Args> 
struct GLstate {
    GLstate() = default;

    template<typename... Args>
    GLstate(Args&&... args) : state{ std::forward<Args>(args)... } {}

    void apply() const {
        GLglobal::instance.get<Args...>() = state;
        state.apply();
    }

    static void save() { GLglobal::saved.get<Args...>() = GLglobal::instance.get<Args...>(); }

    static void restore() {
        auto& saved = GLglobal::saved.get<Args...>();
        GLglobal::instance.get<Args...>() = saved;
        saved.apply();
    }

    void fetch() { state.fetch(); GLglobal::instance.get<Args...>() = state; }

    detail::GLstatefrag<Args...> state = GLglobal::instance.get<Args...>();
};

// simple RAII helper for saving a state within a scope
template<GLenum... Args>
struct GLsavestate {
    GLsavestate() = default; // by default, the state will get the current global state
    ~GLsavestate() { state.apply(); } // when the save state is destructed, the old state will be applied
private:
    GLstate<Args...> state;
};

class GLsavestate_broad {
    struct state {
        GLsavestate<GL_BINDING> bindings;
        GLsavestate<GL_BLEND> blend;
        GLsavestate<GL_DEPTH_TEST> depth;
        GLsavestate<GL_CULL_FACE> cull;
        GLsavestate<GL_VIEWPORT> viewport;
        GLsavestate<GL_SCISSOR_TEST> scissorBox;
    } state;
};