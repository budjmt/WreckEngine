#pragma once

struct GLres { GLres() = default; virtual void update() const = 0; };

template<typename T, bool custom = false>
class GLresource : public GLres {
public:
    GLresource() = default;
    GLresource(GLuniform<T> loc) : location(loc) {}
    GLresource(const GLprogram& p, const char* name) : GLresource(p.getUniform<T>(name)) {}

    T value{};

    void update() const override { location.update(value); }

private:
    GLuniform<T> location;
};

template<typename T>
class GLresource<T, true> : public GLres {
public:
    GLresource() = default;
    GLresource(GLuniform<T> loc, std::function<T()> update) : location(loc), update_func(update) {}
    GLresource(const GLprogram& p, const char* name, std::function<T()> update) : GLresource(p.getUniform<T>(name), update) {}

    void update() const override { location.update(update_func()); }

private:
    GLuniform<T> location;
    std::function<T()> update_func;
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
    GLresource() = default;
    GLresource(GLuniform<vec2> loc) : location(loc) {}
    GLresource(const GLprogram& p, const char* name) : GLresource(p.getUniform<vec2>(name)) {}

    void update() const override { location.update({ Window::width, Window::height }); }

private:
    GLuniform<vec2> location;
};

// Camera resource definitions are in the Render.h
