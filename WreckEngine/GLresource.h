#pragma once

#include <map>

// To use resources with materials, they must be "trivially" copyable and destructible...
// in the sense that they do not have user-defined copy constructors or destructors
// it doesn't really matter that they have v-ptrs in this case because they're just copied data
struct GLres {
    virtual ~GLres() {}
    virtual void update() const = 0; 
};

template<typename T>
class GLresource : public GLres {
public:
    using uniform_t = T;

    GLresource() = default;
    GLresource(GLuniform<T> loc) : location(loc) {}
    GLresource(const GLprogram& p, const char* name) : GLresource(p.getUniform<T>(name)) {}

    static_assert(std::is_trivially_destructible<T>::value && std::is_trivially_copyable<T>::value, "T must be trivially copyable and destructible");
    T value{};

    void update() const override { location.update(value); }
private:
    GLuniform<T> location;
};

struct GLtime;
struct GLresolution;
struct GLcamera {
    struct matrix;
    struct position;
    struct direction;
};

template<>
class GLresource<GLtime> : public GLres {
public:
    using uniform_t = float;

    GLresource() = default;
    GLresource(GLuniform<float> loc) : location(loc) {}
    GLresource(const GLprogram& p, const char* name) : GLresource(p.getUniform<float>(name)) {}

    void update() const override {
        value += (float) Time::delta;
        location.update(value);
    }

private:
    GLuniform<float> location;
    mutable float value = 0;
};

template<>
class GLresource<GLresolution> : public GLres {
public:
    using uniform_t = vec2;

    GLresource() = default;
    GLresource(GLuniform<vec2> loc) : location(loc) {}
    GLresource(const GLprogram& p, const char* name) : GLresource(p.getUniform<vec2>(name)) {}

    void update() const override { location.update({ Window::width, Window::height }); }

private:
    GLuniform<vec2> location;
};

// Camera resource definitions are in the Render.h

namespace std {
    template<>
    struct hash<GLprogram> {
        size_t operator()(const GLprogram& prog) const {
            return std::hash<GLuint>{}(prog.program->id);
        }
    };
}