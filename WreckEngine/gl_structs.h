#pragma once

#include "GL/glew.h"

#include "GLmanager.h"
#include "GLError.h"

#include <vector>

#include "MarchMath.h"
#include "smart_ptr.h"
#include "External.h"

#define local(name) __local__ ## name

typedef GLint GLsampler;

#define WR_GL_OP_PARENS(propType, propName) propType& operator()() const { return *propName; }
#define WR_GL_OP_EQEQ(type, propName) bool operator==(const type& other) const { return *propName == *other.propName; }

inline GLint getGLVal(GLenum value) { GLint val; GL_CHECK(glGetIntegerv(value, &val)); return val; }

#define CHECK_GL_VERSION(major, minor) glewIsSupported("GL_VERSION_" #major "_" #minor)

namespace {
    // default value used to represent "uninitialized" resources
    constexpr GLuint def = (GLuint)-1;

#define GET_GL_CONSTANT_FUNC(name, enumVal) GLint local(name)() { return getGLVal(enumVal); }

    GET_GL_CONSTANT_FUNC(getMaxNumTextures, GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS);
    GET_GL_CONSTANT_FUNC(getMaxColorAttachments, GL_MAX_COLOR_ATTACHMENTS);

    void local(delTexture)    (GLuint* t) { if (GLFWmanager::initialized && *t != def) GL_CHECK(glDeleteTextures(1, t));     delete t; }
    void local(delBuffer)     (GLuint* b) { if (GLFWmanager::initialized && *b != def) GL_CHECK(glDeleteBuffers(1, b));      delete b; }
    void local(delVAO)        (GLuint* a) { if (GLFWmanager::initialized && *a != def) GL_CHECK(glDeleteVertexArrays(1, a)); delete a; }
    void local(delShader)     (GLuint* s) { if (GLFWmanager::initialized && *s != def) GL_CHECK(glDeleteShader(*s));         delete s; }
    void local(delShaderProg) (GLuint* p) { if (GLFWmanager::initialized && *p != def) GL_CHECK(glDeleteProgram(*p));        delete p; }
    void local(delFrameBuffer)(GLuint* f) { if (GLFWmanager::initialized && *f != def) GL_CHECK(glDeleteFramebuffers(1, f)); delete f; }
}

struct GLtexture;
struct GLbuffer;
struct GLVAO;
struct GLshader;
struct GLprogram;
struct GLframebuffer;

struct GLdepthstencil;

// maps primitive types to their matching GLenum values, (if applicable) or returns GL_FALSE.
template<typename T> inline constexpr GLenum GLtype()  { return GL_FALSE; }
template<> inline constexpr GLenum GLtype<GLbyte>()    { return GL_BYTE; }
template<> inline constexpr GLenum GLtype<GLubyte>()   { return GL_UNSIGNED_BYTE; }
template<> inline constexpr GLenum GLtype<GLshort>()   { return GL_SHORT; }
template<> inline constexpr GLenum GLtype<GLushort>()  { return GL_UNSIGNED_SHORT; }
template<> inline constexpr GLenum GLtype<GLint>()     { return GL_INT; }
template<> inline constexpr GLenum GLtype<GLuint>()    { return GL_UNSIGNED_INT; }
template<> inline constexpr GLenum GLtype<GLfloat>()   { return GL_FLOAT; }
template<> inline constexpr GLenum GLtype<GLdouble>()  { return GL_DOUBLE; }

template<> inline constexpr GLenum GLtype<GLdepthstencil>() { return GL_UNSIGNED_INT_24_8; }

template<> inline constexpr GLenum GLtype<GLtexture>()     { return GL_TEXTURE; }
template<> inline constexpr GLenum GLtype<GLbuffer>()      { return GL_BUFFER; }
template<> inline constexpr GLenum GLtype<GLVAO>()         { return GL_VERTEX_ARRAY; }
template<> inline constexpr GLenum GLtype<GLshader>()      { return GL_SHADER; }
template<> inline constexpr GLenum GLtype<GLprogram>()     { return GL_PROGRAM; }
template<> inline constexpr GLenum GLtype<GLframebuffer>() { return GL_FRAMEBUFFER; }

// wraps a location pointing to a uniform variable of type T. the value is updated using update(T t). If there is no definition for update, the type is unsupported.
template<typename T> struct GLuniform { GLuint location; };
template<> struct GLuniform<GLint>     { GLuint location; inline void update(GLint value)       const { GL_CHECK(glUniform1i(location, value)); }; };
template<> struct GLuniform<GLuint>    { GLuint location; inline void update(GLuint value)      const { GL_CHECK(glUniform1ui(location, value)); }; };
template<> struct GLuniform<GLfloat>   { GLuint location; inline void update(GLfloat value)     const { GL_CHECK(glUniform1f(location, value)); }; };
template<> struct GLuniform<GLdouble>  { GLuint location; inline void update(GLdouble value)    const { GL_CHECK(glUniform1d(location, value)); }; };
template<> struct GLuniform<GLboolean> { GLuint location; inline void update(GLboolean value)   const { GL_CHECK(glUniform1i(location, value)); }; };
template<> struct GLuniform<vec2>      { GLuint location; inline void update(const vec2& value) const { GL_CHECK(glUniform2fv(location, 1, &value[0])); }; };
template<> struct GLuniform<vec3>      { GLuint location; inline void update(const vec3& value) const { GL_CHECK(glUniform3fv(location, 1, &value[0])); }; };
template<> struct GLuniform<vec4>      { GLuint location; inline void update(const vec4& value) const { GL_CHECK(glUniform4fv(location, 1, &value[0])); }; };
template<> struct GLuniform<mat3>      { GLuint location; inline void update(const mat3& value, bool transpose = false) const { GL_CHECK(glUniformMatrix3fv(location, 1, transpose, &value[0][0])); }; };
template<> struct GLuniform<mat4>      { GLuint location; inline void update(const mat4& value, bool transpose = false) const { GL_CHECK(glUniformMatrix4fv(location, 1, transpose, &value[0][0])); }; };

// wraps a texture. [type] reflects what type of sampler it needs.
// https://www.opengl.org/wiki/Sampler_(GLSL)
struct GLtexture {
    shared<GLuint> texture = shared<GLuint>(new GLuint(def), local(delTexture));
    GLenum type;
    inline WR_GL_OP_PARENS(GLuint, texture);
    inline WR_GL_OP_EQEQ(GLtexture, texture);

    inline bool valid() const { return *texture != def; }

    inline void create(const GLenum _type = GL_TEXTURE_2D, const GLint maxMipLevel = 0) {
        if (valid()) {
            return;
        }
        type = _type;
        GL_CHECK(glGenTextures(1, texture.get()));
        // these are globally bound, so technically this line affects every texture [of the type] each time the value changes
        // this can be fixed with immutable textures in 4.3+
        param(GL_TEXTURE_BASE_LEVEL, 0);
        param(GL_TEXTURE_MAX_LEVEL, maxMipLevel);
    }
    // must be done while bound
    inline void genMipMap() {
        GL_CHECK(glGenerateMipmap(type));
    }
    inline void bind(const GLint index = 0) const {
        assert(index < MAX_TEXTURES);
        GL_CHECK(glActiveTexture(GL_TEXTURE0 + index));
        GL_CHECK(glBindTexture(type, *texture));
    }
    inline void unbind() {
        GL_CHECK(glBindTexture(type, 0));
    }

    inline void unload() {
        if (*texture != def) {
            GL_CHECK(glDeleteTextures(1, texture.get()));
            *texture = def;
        }
    }

    inline void param(const GLenum name, const int val) {
        GL_CHECK(glTexParameteri(type, name, val));
    }
    inline void param(const GLenum name, const float val) {
        GL_CHECK(glTexParameterf(type, name, val));
    }

    // these sets correspond to glTexImage. Texture must be bound for these to work.
    template<typename value_t>
    inline void set1D(const GLvoid* pixelData, const GLuint width, const GLenum formatFrom = GL_RGBA, const GLenum formatTo = GL_RGBA, const GLint mipLevel = 0) const {
        GL_CHECK(glTexImage1D(type, mipLevel, formatTo, width, 0, formatFrom, GLtype<value_t>(), pixelData));
    }
    template<typename value_t>
    inline void set2D(const GLvoid* pixelData, const GLuint width, const GLuint height, const GLenum formatFrom = GL_RGBA, const GLenum formatTo = GL_RGBA, const GLint mipLevel = 0) const {
        GL_CHECK(glTexImage2D(type, mipLevel, formatTo, width, height, 0, formatFrom, GLtype<value_t>(), pixelData));
    }
    template<typename value_t>
    inline void setSub2D(const GLvoid* pixelData, const GLint xoffset, const GLint yoffset, const GLuint width, const GLuint height, const GLenum format = GL_RGBA, const GLint mipLevel = 0) const {
        GL_CHECK(glTexSubImage2D(type, mipLevel, xoffset, yoffset, width, height, format, GLtype<value_t>(), pixelData));
    }
    template<typename value_t>
    inline void set3D(const GLvoid* pixelData, const GLuint width, const GLuint height, const GLuint depth, const GLenum formatFrom = GL_RGBA, const GLenum formatTo = GL_RGBA, const GLint mipLevel = 0) const {
        GL_CHECK(glTexImage3D(type, mipLevel, formatTo, width, height, depth, 0, formatFrom, GLtype<value_t>(), pixelData));
    }

private:
    static GLint MAX_TEXTURES;
    static void setMaxTextures() {
        MAX_TEXTURES = local(getMaxNumTextures)();
    }
    friend struct GLFWmanager;
};

// wraps a buffer object on the GPU.
struct GLbuffer {
    shared<GLuint> buffer {new GLuint(def), local(delBuffer)};
    GLenum target, usage;
    size_t size = 0;
    inline WR_GL_OP_PARENS(GLuint, buffer);

    inline bool valid() const { return *buffer != def; }

    inline GLint getVal(GLenum value) const {
        GLint val;
        GL_CHECK(glGetBufferParameteriv(target, value, &val));
        return val;
    }

    inline void set(const GLenum target, const GLenum usage) {
        this->target = target; this->usage = usage;
    }

    // call this to replace the currently stored buffer index with a newly created one.
    // target: GL_ARRAY_BUFFER or GL_ELEMENT_ARRAY_BUFFER
    // usage: GL_STATIC_DRAW by default, also can be GL_STREAM_DRAW or GL_DYNAMIC_DRAW
    inline void create(const GLenum target, const GLenum usage = GL_STATIC_DRAW) {
        if (valid()) return;
        GL_CHECK(glGenBuffers(1, buffer.get()));
        set(target, usage);
    }

    // allows for binding buffer as alternative target
    inline void bindAs(GLenum _target) const {
        GL_CHECK(glBindBuffer(_target, *buffer));
    }

    // call to bind the buffer to its target
    inline void bind() const {
        bindAs(target);
    }
    inline void unbind() {
        GL_CHECK(glBindBuffer(target, 0));
    }

    // binds to an index, which resizes when the buffer is resized
    inline void bindBase(GLuint index) const {
        GL_CHECK(glBindBufferBase(target, index, *buffer));
    }
    inline void unbindBase(GLuint index) const {
        GL_CHECK(glBindBufferBase(target, index, 0));
    }

    // binds to an index with an optional offset, must be manually resized
    inline void bindRange(GLuint index, GLintptr offset = 0) const {
        GL_CHECK(glBindBufferRange(target, index, *buffer, offset, size));
    }
    inline void unbindRange(GLuint index) const {
        unbindBase(index); // equivalent operation, provided for convenience
    }

    // invalidates the buffer; used for streaming
    inline void invalidate() const {
        GL_CHECK(glBufferData(target, size, nullptr, usage));
    }

    // allocate the buffer after binding to contain [size] bytes from [_data].
    // IMPORTANT: the size of the array [_data] points to must match [size], or there will be access exceptions
    // calling these methods from an unbound buffer is allowed, but will have undefined results
    inline void data(const size_t size, const GLvoid* _data) {
        GL_CHECK(glBufferData(target, size, _data, usage));
        this->size = size;
    }

    // this version is intended for updates, (for streams) not instantiations
    inline void data(const GLvoid* _data) const {
        assert(usage != GL_STATIC_DRAW && size); // a buffer allocated with static draw should not be updated / a buffer of 0 size should not need updates
        invalidate();
        GL_CHECK(glBufferData(target, size, _data, usage));
    }

    // updates a subset of a buffer's data
    inline void subdata(const GLvoid* data, const GLuint _size, const GLuint offset = 0) const {
        assert(usage != GL_STATIC_DRAW && size); // a buffer allocated with static draw should not be updated / a buffer of size 0 shouldn't need updates
        GL_CHECK(glBufferSubData(target, offset, _size, data));
    }
};

// wraps a VAO, stores bindings for attributes and buffers after binding
struct GLVAO {
    shared<GLuint> vao {new GLuint(def), local(delVAO)};
    inline WR_GL_OP_PARENS(GLuint, vao);

    inline bool valid() const { return *vao != def; }

    inline void create() const {
        if (valid()) return; 
        GL_CHECK(glGenVertexArrays(1, vao.get()));
    }
    inline void bind() const {
        GL_CHECK(glBindVertexArray(*vao));
    }
    static inline void unbind() {
        GL_CHECK(glBindVertexArray(0));
    }
};

// a uniform block variable in GLSL, 
template<typename T>
struct GLuniformblock : public GLuniform<T> {
    GLuint index;
    GLbuffer block;

    inline void create() { block.create(GL_UNIFORM_BUFFER, GL_STREAM_DRAW); }
    inline void bind(const size_t offsetBytes = 0) const { block.bindRange(index, offsetBytes); }

    inline void update(const size_t size, const T* data) {
        block.bind();
        block.invalidate();
        block.data(size, data);
    }
};

// wraps a compiled shader
struct GLshader {
    GLenum type = def;
    inline WR_GL_OP_PARENS(const GLuint, shader);

    inline bool valid() const { return *shader != def; }

    // creates and compiles a shader of [type] from [body] and stores it
    inline void create(const char* body, const GLenum type) {
        if (valid()) return;
        this->type = type;
        GL_CHECK(*shader = glCreateShader(type));
        GL_CHECK(glShaderSource(*shader, 1, &body, 0));
        GL_CHECK(glCompileShader(*shader));
    }
private:
    shared<GLuint> shader {new GLuint(def), local(delShader)};
    friend struct GLprogram;
};

// wraps a shader program [linked from several shaders]
struct GLprogram {
    GLshader vertex, tessControl, tessEval, geometry, fragment;
    shared<GLuint> program {new GLuint(def), local(delShaderProg)};
    inline WR_GL_OP_PARENS(GLuint, program);

    inline bool valid() const { return *program != def; }

    inline void create() {
        if (valid()) return;
        GL_CHECK(*program = glCreateProgram());
    }

    inline void attach(const GLshader& shader) const { GL_CHECK(glAttachShader(*program, shader())); }

    // properly sets up the program once the shaders are set
    inline void link() const {
        if (vertex.valid())       attach(vertex);
        if (tessControl.valid())  attach(tessControl);
        if (tessEval.valid())     attach(tessEval);
        if (geometry.valid())     attach(geometry);
        if (fragment.valid())     attach(fragment);
        GL_CHECK(glLinkProgram(*program));
    }

    inline void use() const {
        GL_CHECK(glUseProgram(*program));
    }
    
    // used for retrieving uniform variables (of type T)
    template<typename T> 
    inline GLuniform<T> getUniform(const char* name) const {
        GLuniform<T> u;
        GL_CHECK(u.location = glGetUniformLocation(*program, name));
        return u;
    }

    // binds a uniform block index to a slot index (must be called before use)
    inline void bindUniformBlock(const uint32_t location, const uint32_t index) const {
        GL_CHECK(glUniformBlockBinding(*program, location, index));
    }

    // used for retrieving uniform block variables (of type T)
    template<typename T>
    inline GLuniformblock<T> getUniformBlock(const char* name) const {
        GLuniformblock<T> u;
        GL_CHECK(u.location = glGetUniformBlockIndex(*program, name));
        return u;
    }

    // used for retrieving uniform block variables (of type T)
    template<typename T>
    inline GLuniformblock<T> getUniformBlock(const char* name, const uint32_t index) const {
        auto u = getUniformBlock<T>(name);
        bindUniformBlock(u.location, u.index = index);
        return u;
    }

    // convenience function for setting a one time uniform value, e.g. a GLsampler 
    template<typename T> 
    inline void setOnce(const char* name, const T& value) const { getUniform<T>(name).update(value); }

    inline size_t getUniformOffset(const char* name) {
        GLuint index;
        GL_CHECK(index = glGetProgramResourceIndex(*program, GL_UNIFORM, name));

        GLenum prop = GL_OFFSET;
        GLint length, param;
        GL_CHECK(glGetProgramResourceiv(*program, GL_UNIFORM, index, 1, &prop, sizeof(GLuint), &length, &param));
        return param;
    }
};

// allows access to frame buffers
// For a frame buffer to be valid, it must meet the following requirements:
//   - Attached to at least one buffer (color, depth, stencil, etc.)
//   - Attached to at least one _color_
//   - All attachments are complete
//   - All attachments have the same number of multi-samples
struct GLframebuffer {
    shared<GLuint> framebuffer{ new GLuint(def), local(delFrameBuffer) };
    GLenum type = GL_FRAMEBUFFER;
    inline WR_GL_OP_PARENS(GLuint, framebuffer);

    inline bool valid() const { return *framebuffer != def; }

    inline GLenum check() const { GLenum res; GL_CHECK(res = glCheckFramebufferStatus(type)); return res; }
    inline bool checkComplete() const { return check() == GL_FRAMEBUFFER_COMPLETE; }
    inline bool isBound() const { return *framebuffer == boundFBO; }
    inline size_t numOutputs() const { return colorBuffers.size(); }

    static inline vec4 getClearColor() {
        vec4 color;
        GL_CHECK(glGetFloatv(GL_COLOR_CLEAR_VALUE, &color.r));
        return color;
    }

    static inline void setClearColor(GLclampf r, GLclampf g, GLclampf b, GLclampf a) { GL_CHECK(glClearColor(r, g, b, a)); }

    static inline void clear() { GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT)); }
    
    static inline void clearPartial(GLenum buffer, GLint drawBuffer, const GLfloat* value) { 
        assert(buffer == GL_COLOR || drawBuffer == 0); 
        GL_CHECK(glClearBufferfv(buffer, drawBuffer, value)); 
    }

    static inline void clearPartial(GLfloat depth = 1.f, GLint stencil = 0) { 
        GL_CHECK(glClearBufferfi(GL_DEPTH_STENCIL, 0, depth, stencil)); 
    }

    inline void create(const GLenum target = GL_FRAMEBUFFER) {
        if (valid()) return;
        GL_CHECK(glGenFramebuffers(1, framebuffer.get()));
        type = target;
    }
    
    inline void bindPartial() const { 
        boundFBO = *framebuffer; 
        GL_CHECK(glBindFramebuffer(type, *framebuffer)); 
    }

    static inline void setDrawBuffers(const size_t size, const GLenum* drawBuffers) {
        GL_CHECK(glDrawBuffers(size, drawBuffers));
    }

    inline void bind() const { 
        bindPartial();
        setDrawBuffers(colorBuffers.size(), &colorBuffers[0]); 
    }

    enum Attachment : GLenum { 
        Color        = GL_COLOR_ATTACHMENT0, 
        Depth        = GL_DEPTH_ATTACHMENT, 
        Stencil      = GL_STENCIL_ATTACHMENT, 
        DepthStencil = GL_DEPTH_STENCIL_ATTACHMENT 
    };
    // attaches a texture to the frame buffer for the specified output.
    // note that textures attached to depth and/or stencil should use the correct internal format, e.g. GL_DEPTH24_STENCIL8
    void attachTexture(const GLtexture& tex, Attachment attachment) {
        assert((GLint)(colorBuffers.size() + 1) < MAX_COLOR_ATTACHMENTS);

        if (attachment == Attachment::Color) {
            GL_CHECK(glFramebufferTexture2D(type, attachment + colorBuffers.size(), tex.type, tex(), 0));
            colorBuffers.push_back(attachment + colorBuffers.size());
        }
        else
            GL_CHECK(glFramebufferTexture2D(type, attachment, tex.type, tex(), 0));
    }

    void rebindTexture(const GLtexture& tex, Attachment attachment, const uint32_t index = 0) {
        assert(attachment == Attachment::Color || index == 0); // everything except Color can't have multiple attachments
        assert(index <= colorBuffers.size()); // can't rebind what hasn't been bound

        GL_CHECK(glFramebufferTexture2D(type, attachment + index, tex.type, tex(), 0));
    }

    void unbind() const { unbind(type); }
    static void unbind(GLenum type) { boundFBO = 0; GL_CHECK(glBindFramebuffer(type, 0)); }

    // reads [width] x [height] pixels from the frame buffer starting from [x],[y] (lower-left corner) into [dest]
    // relevant to this function: GL_PIXEL_PACK_BUFFER and glPixelStore
    // in addition to the traditional color formats, [format] can accept GL_DEPTH_COMPONENT, GL_STENCIL_INDEX, GL_DEPTH_STENCIL to read those buffers
    // in the case of GL_STENCIL_INDEX, if the [type] is not GL_FLOAT, there will be masking: https://www.opengl.org/wiki/GLAPI/glReadPixels#Description
    void readPixels(void* dest, const int x, const int y, const size_t width, const size_t height, const GLenum texFormat = GL_RGBA, const GLenum unpackType = GLtype<uint32_t>()) const {
        assert(type == GL_FRAMEBUFFER || type == GL_READ_FRAMEBUFFER); // the frame buffer must be bound to GL_READ_FRAMEBUFFER (or GL_FRAMEBUFFER) for this operation to succeed
        assert(texFormat != GL_DEPTH_STENCIL || unpackType == GL_UNSIGNED_INT_24_8 || unpackType == GL_FLOAT_32_UNSIGNED_INT_24_8_REV); // GL_DEPTH_STENCIL has special requirements
        GL_CHECK(glReadPixels(x, y, width, height, texFormat, unpackType, dest));
    }

    // creates a render target. If this is for depth and/or stencil, [from] must be GL_DEPTH_COMPONENT, GL_STENCIL_INDEX, or GL_DEPTH_STENCIL
    template<typename T>
    static GLtexture createRenderTarget(GLenum to = GL_RGBA, GLenum from = GL_RGBA) {
        GLtexture tex;
        tex.create();
        tex.bind();
        tex.param(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        tex.param(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        tex.set2D<T>(nullptr, Window::width, Window::height, from, to);
        return tex;
    }

private:
    std::vector<GLenum> colorBuffers;
    static GLint boundFBO;

    // if errors persist, GL_MAX_DRAW_BUFFERS is how many buffers can be drawn to per draw call
    static GLint MAX_COLOR_ATTACHMENTS;
    static void setMaxColorAttachments() {
        MAX_COLOR_ATTACHMENTS = local(getMaxColorAttachments)();
    }
    friend struct GLFWmanager;
};

struct GLres { virtual void update() const = 0; };

template<typename T>
class GLresource : public GLres {
public:
    GLresource() = default;
    GLresource(const GLuniform<T> loc) : location(loc) {}
    GLresource(const GLprogram& p, const char* name) : location(p.getUniform<T>(name)) {}
    
    T value;

    void update() const override { location.update(value); }

private:
    GLuniform<T> location;
};

struct GLtime;
struct GLresolution;

template<>
class GLresource<GLtime> : public GLres {
public:
    GLresource() = default;
    GLresource(const GLuniform<float> loc) : location(loc) {}
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
    GLresource(const GLuniform<vec2> loc) : location(loc) {}
    GLresource(const GLprogram& p, const char* name) : location(p.getUniform<vec2>(name)) {}

    void update() const override { location.update(vec2(Window::width, Window::height)); }

private:
    GLuniform<vec2> location;
};

// helper class, used to assist in creating vertex array attribute bindings (create and use locally, DO NOT STORE!)
class GLattrarr {
    struct GLattr {
        GLenum type; GLuint size, bytes, divisor; bool normalize, castToFloat;
    };
    inline void reset() {
        attrs = std::vector<GLattr>();
    }

public:
    std::vector<GLattr> attrs;

    // adds an attribute of type T to the cache. Use a divisor value of 1 for instanced variables
    template<typename T>
    inline void add(const size_t size, const GLuint divisor = 0, const bool normalize = GL_FALSE, const bool castToFloat = false) {
        GLattr attr;
        attr.type = GLtype<T>();
        attr.size = size;
        attr.divisor = divisor;
        attr.bytes = sizeof(T) * size;
        attr.normalize = normalize;
        attr.castToFloat = normalize || castToFloat;

        attrs.push_back(attr);
    }

    template<> inline void add<vec2>(const size_t size, const GLuint divisor, const bool normalize, const bool castToFloat) {
        for (size_t i = 0; i < size; ++i) add<GLfloat>(2, divisor, normalize, castToFloat);
    }
    template<> inline void add<vec3>(const size_t size, const GLuint divisor, const bool normalize, const bool castToFloat) {
        for (size_t i = 0; i < size; ++i) add<GLfloat>(3, divisor, normalize, castToFloat);
    }
    template<> inline void add<vec4>(const size_t size, const GLuint divisor, const bool normalize, const bool castToFloat) {
        for (size_t i = 0; i < size; ++i) add<GLfloat>(4, divisor, normalize, castToFloat);
    }
    template<> inline void add<mat3>(const size_t size, const GLuint divisor, const bool normalize, const bool castToFloat) {
        for (size_t i = 0; i < size; ++i) {
            add<GLfloat>(3, divisor, normalize, castToFloat);
            add<GLfloat>(3, divisor, normalize, castToFloat);
            add<GLfloat>(3, divisor, normalize, castToFloat);
        }
    }
    template<> inline void add<mat4>(const size_t size, const GLuint divisor, const bool normalize, const bool castToFloat) {
        for (size_t i = 0; i < size; ++i) {
            add<GLfloat>(4, divisor, normalize, castToFloat);
            add<GLfloat>(4, divisor, normalize, castToFloat);
            add<GLfloat>(4, divisor, normalize, castToFloat);
            add<GLfloat>(4, divisor, normalize, castToFloat);
        }
    }

    // call once all desired attributes have been added; applies the added attributes, starting at [baseIndex] and clears the cache.
    // [baseIndex] would be used when there are multiple buffers bound to a single shader, e.g. instanced rendering, where the stride must be reset
    inline GLuint apply(const GLuint baseIndex = 0, const size_t endPadding = 0, const size_t startOffset = 0) {
        size_t stride = endPadding;
        for (const auto& attr : attrs) {
            stride += attr.bytes;
        }
        auto offset = (char*) startOffset;
        for (size_t i = 0, entries = attrs.size(); i < entries; ++i) {
            const auto attr = attrs[i];
            GL_CHECK(glEnableVertexAttribArray(i + baseIndex));
            if (attr.castToFloat) {
            FLOATCAST: 
                GL_CHECK(glVertexAttribPointer(i + baseIndex, attr.size, attr.type, attr.normalize, stride, offset));
            }
            else {
                switch (attr.type) {
                case GLtype<GLbyte>() :
                case GLtype<GLubyte>() :
                case GLtype<GLshort>() :
                case GLtype<GLushort>() :
                case GLtype<GLint>() :
                case GLtype<GLuint>() :
                    GL_CHECK(glVertexAttribIPointer(i + baseIndex, attr.size, attr.type, stride, offset));
                    break;
                case GLtype<GLdouble>():
                    GL_CHECK(glVertexAttribLPointer(i + baseIndex, attr.size, attr.type, stride, offset));
                    break;
                default:
                    goto FLOATCAST;
                }
            }
            if (attr.divisor)
            {
                GL_CHECK(glVertexAttribDivisor(i + baseIndex, attr.divisor));
            }
            offset += attr.bytes;
        }
        GLuint finalIndex = attrs.size() + baseIndex;
        reset();
        return finalIndex;
    }

private:
    static bool isIntType(GLenum type) {
        switch (type) {
            case GL_BYTE:
            case GL_UNSIGNED_BYTE:
            case GL_SHORT:
            case GL_UNSIGNED_SHORT:
            case GL_INT:
            case GL_UNSIGNED_INT:
                return true;
            default:
                return false;
        }
    }
};
