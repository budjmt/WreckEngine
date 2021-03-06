#pragma once

#include "GL/glew.h"

#include "GLmanager.h"
#include "GLError.h"

#include <vector>

#include "MarchMath.h"
#include "smart_ptr.h"
#include "External.h"

#include "GLstate.h"

#define local(name) __local__ ## name

typedef GLint GLsampler;

#define WR_GL_OP_PARENS(handleName) GLuint& operator()() { return handleName->id; } WR_GL_OP_PARENS_CONST(handleName)
#define WR_GL_OP_PARENS_CONST(handleName) const GLuint& operator()() const { return handleName->id; }

#define WR_GL_OP_EQEQ(type, handleName) bool operator==(const type& other) const { return handleName->id == other.handleName->id; }
#define WR_GL_RAW_OP_EQEQ(type, handleName) \
inline bool operator==(const type& it, GLuint id) { return it.handleName->id == id; } \
inline bool operator==(GLuint id, const type& it) { return it == id; }

#define CHECK_GL_VERSION(major, minor) glewIsSupported("GL_VERSION_" #major "_" #minor)

template<typename T>
struct GLhandle {
    GLuint id = def;
    GLhandle() = default;
    explicit GLhandle(GLuint _id) : id(_id) {}
    ~GLhandle() { 
        if (GLFWmanager::initialized && id != def)
            T::deleter(id);
    }
    GLhandle(const GLhandle&) = default;
    GLhandle& operator=(const GLhandle&) = default;
    GLhandle(GLhandle&&) = default;
    GLhandle& operator=(GLhandle&&) = default;

    inline bool operator==(const GLhandle& other) { return id == other.id; }
    inline bool valid() const { return id != def; } 
};

namespace {
    // default value used to represent "uninitialized" resources
    constexpr GLuint def = (GLuint)-1;

#define GET_GL_CONSTANT_FUNC(name, enumVal) GLint local(name)() { return getGLVal(enumVal); }

    GET_GL_CONSTANT_FUNC(getMaxNumTextures, GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS);
    GET_GL_CONSTANT_FUNC(getMaxColorAttachments, GL_MAX_COLOR_ATTACHMENTS);
    GET_GL_CONSTANT_FUNC(getMaxVertexAttribs, GL_MAX_VERTEX_ATTRIBS);
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
struct GLuniform_t { GLuint location; };
template<typename T> struct GLuniform  : public GLuniform_t {};
template<> struct GLuniform<GLint>     : public GLuniform_t { inline void update(GLint value)       const { GL_CHECK(glUniform1i(location, value)); }; };
template<> struct GLuniform<GLuint>    : public GLuniform_t { inline void update(GLuint value)      const { GL_CHECK(glUniform1ui(location, value)); }; };
template<> struct GLuniform<GLfloat>   : public GLuniform_t { inline void update(GLfloat value)     const { GL_CHECK(glUniform1f(location, value)); }; };
template<> struct GLuniform<GLdouble>  : public GLuniform_t { inline void update(GLdouble value)    const { GL_CHECK(glUniform1d(location, value)); }; };
template<> struct GLuniform<GLboolean> : public GLuniform_t { inline void update(GLboolean value)   const { GL_CHECK(glUniform1i(location, value)); }; };
template<> struct GLuniform<vec2>      : public GLuniform_t { inline void update(const vec2& value) const { GL_CHECK(glUniform2fv(location, 1, &value[0])); }; };
template<> struct GLuniform<vec3>      : public GLuniform_t { inline void update(const vec3& value) const { GL_CHECK(glUniform3fv(location, 1, &value[0])); }; };
template<> struct GLuniform<vec4>      : public GLuniform_t { inline void update(const vec4& value) const { GL_CHECK(glUniform4fv(location, 1, &value[0])); }; };
template<> struct GLuniform<mat3>      : public GLuniform_t { inline void update(const mat3& value, bool transpose = false) const { GL_CHECK(glUniformMatrix3fv(location, 1, transpose, &value[0][0])); }; };
template<> struct GLuniform<mat4>      : public GLuniform_t { inline void update(const mat4& value, bool transpose = false) const { GL_CHECK(glUniformMatrix4fv(location, 1, transpose, &value[0][0])); }; };

// wraps a texture. [type] reflects what type of sampler it needs.
// https://www.opengl.org/wiki/Sampler_(GLSL)
struct GLtexture {
    using handle_t = GLhandle<GLtexture>;
    shared<handle_t> texture = make_shared<handle_t>();
    GLenum target = def;
    inline WR_GL_OP_PARENS(texture);
    inline WR_GL_OP_EQEQ(GLtexture, texture);

    static GLint getFormatPitch(GLenum format);

    static void deleter(const GLuint& id) { GL_CHECK(glDeleteTextures(1, &id)); }
    inline bool valid() const { return texture->valid(); }

    inline void create(const GLenum _type = GL_TEXTURE_2D, const GLint maxMipLevel = 0) {
        if (valid()) return;
        target = _type;
        GL_CHECK(glGenTextures(1, &texture->id));
        // these are globally bound, so technically this line affects every texture [of the type] each time the value changes
        // this can be fixed with immutable textures in 4.3+
        param(GL_TEXTURE_BASE_LEVEL, 0);
        param(GL_TEXTURE_MAX_LEVEL, maxMipLevel);
    }
    // must be done while bound
    inline void genMipMap() {
        GL_CHECK(glGenerateMipmap(target));
    }

    inline void bind(const GLint index = 0) const {
        assert(index < MAX_TEXTURES);
        GLstate<GL_BINDING, GL_ACTIVE_TEXTURE>{ GL_TEXTURE0 + index }.apply();
        bind_helper(target, texture->id);
    }
    inline void unbind() {
        bind_helper(target, 0);
    }

    inline void bindImage(const GLenum access = GL_READ_WRITE, const GLenum format = GL_RGBA, const GLint index = 0, const GLuint level = 0, const GLboolean layered = GL_FALSE, const GLint layer = 0) {
        // don't need to make the texture active to bind an image
        GL_CHECK(glBindImageTexture(index, texture->id, level, layered, layer, access, format)); // doesn't support 3D/array/layered textures right now
    }
    inline void unbindImage(const GLint index = 0) {
        GL_CHECK(glBindImageTexture(index, 0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGB)); // values besides index and the first 0 are irrelevant
    }

    inline void unload() {
        texture->~GLhandle();
        texture->id = def;
    }

    // last value must be an int or float
    template<typename... Args>
    inline void param(GLenum name1, GLenum name2, Args&&... args) {
        auto tup = std::make_tuple(std::forward<Args>(args)...);

        auto val = std::get<sizeof...(args) - 1>(tup);
        using val_t = std::decay_t<decltype(val)>;
        static_assert(std::is_same<val_t, int>::value || std::is_same<val_t, float>::value, "Texture parameter values must be int or float");

        param(name1, val);
        param(name2, std::forward<Args>(args)...); // UB, we've already moved out of the parameter pack; switch to std::apply in C++17?
    }

    inline void param(GLenum name, int val) {
        GL_CHECK(glTexParameteri(target, name, val));
    }
    inline void param(GLenum name, float val) {
        GL_CHECK(glTexParameterf(target, name, val));
    }

    inline void set1D(const GLenum type, const void* pixelData, const GLuint width, const GLenum formatFrom, const GLenum formatTo, const GLint mipLevel = 0) const {
        GL_CHECK(glTexImage1D(target, mipLevel, formatTo, width, 0, formatFrom, type, pixelData));
    }
    inline void set2D(const GLenum type, const void* pixelData, const GLuint width, const GLuint height, const GLenum formatFrom, const GLenum formatTo, const GLint mipLevel = 0) const {
        set2DAs(target, type, pixelData, width, height, formatFrom, formatTo, mipLevel);
    }
    inline void set2DAs(const GLenum _target, const GLenum type, const void* pixelData, const GLuint width, const GLuint height, const GLenum formatFrom, const GLenum formatTo, const GLint mipLevel = 0) const {
        GL_CHECK(glTexImage2D(_target, mipLevel, formatTo, width, height, 0, formatFrom, type, pixelData));
    }
    inline void set3D(const GLenum type, const void* pixelData, const GLuint width, const GLuint height, const GLuint depth, const GLenum formatFrom, const GLenum formatTo, const GLint mipLevel = 0) const {
        GL_CHECK(glTexImage3D(target, mipLevel, formatTo, width, height, depth, 0, formatFrom, type, pixelData));
    }

    // these sets correspond to glTexImage. Texture must be bound for these to work.
    template<typename value_t>
    inline void set1D(const GLvoid* pixelData, const GLuint width, const GLenum formatFrom = GL_RGBA, const GLenum formatTo = GL_RGBA, const GLint mipLevel = 0) const {
        set1D(GLtype<value_t>(), pixelData, width, formatFrom, formatTo, mipLevel);
    }
    template<typename value_t>
    inline void set2D(const GLvoid* pixelData, const GLuint width, const GLuint height, const GLenum formatFrom = GL_RGBA, const GLenum formatTo = GL_RGBA, const GLint mipLevel = 0) const {
        set2D(GLtype<value_t>(), pixelData, width, height, formatFrom, formatTo, mipLevel);
    }
    template<typename value_t>
    inline void set3D(const GLvoid* pixelData, const GLuint width, const GLuint height, const GLuint depth, const GLenum formatFrom = GL_RGBA, const GLenum formatTo = GL_RGBA, const GLint mipLevel = 0) const {
        set3D(GLtype<value_t>(), pixelData, width, height, depth, formatFrom, formatTo, mipLevel);
    }

    template<typename value_t>
    inline void setSub2D(const GLvoid* pixelData, const GLint xoffset, const GLint yoffset, const GLuint width, const GLuint height, const GLenum format = GL_RGBA, const GLint mipLevel = 0) const {
        GL_CHECK(glTexSubImage2D(target, mipLevel, xoffset, yoffset, width, height, format, GLtype<value_t>(), pixelData));
    }

    // these sets correspond to glTexImage. Texture must be bound for these to work.
    template<typename value_t>
    inline void setStorage1D(const GLuint width, const GLenum format = GL_RGBA, const GLint mipLevels = 1) const {
        GL_CHECK(glTexStorage1D(target, mipLevels, format, width, GLtype<value_t>()));
    }
    template<typename value_t>
    inline void setStorage2D(const GLuint width, const GLuint height, const GLenum format = GL_RGBA, const GLint mipLevels = 1) const {
        GL_CHECK(glTexStorage2D(target, mipLevels, format, width, height, GLtype<value_t>()));
    }
    template<typename value_t>
    inline void setStorage3D(const GLuint width, const GLuint height, const GLuint depth, const GLenum format = GL_RGBA, const GLint mipLevels = 1) const {
        GL_CHECK(glTexStorage3D(target, mipLevels, formatTo, width, height, depth, GLtype<value_t>()));
    }

    inline void view(const GLtexture& tex, const GLenum format = GL_RGBA, const GLint mipLevels = 1, const GLint baseMip = 0) {
        GL_CHECK(glTextureView(texture->id, target, tex(), format, baseMip, mipLevels, 0, 1));
    }

private:
    static GLint MAX_TEXTURES;
    static void setMaxTextures() {
        MAX_TEXTURES = local(getMaxNumTextures)();
    }

    static void bind_helper(GLenum target, GLint val) {
        switch (target) {
        case GL_TEXTURE_1D:
            GLstate<GL_BINDING, GL_TEXTURE_BINDING_1D>{ val }.apply();
            break;
        case GL_TEXTURE_2D:
            GLstate<GL_BINDING, GL_TEXTURE_BINDING_2D>{ val }.apply();
            break;
        case GL_TEXTURE_3D:
            GLstate<GL_BINDING, GL_TEXTURE_BINDING_3D>{ val }.apply();
            break;
        default:
            GL_CHECK(glBindTexture(target, val));
        }
    }
    friend struct GLEWmanager;
};

// wraps a buffer object on the GPU.
struct GLbuffer {
    using handle_t = GLhandle<GLbuffer>;
    shared<handle_t> buffer = make_shared<handle_t>();
    GLenum target, usage;
    size_t size = 0;
    inline WR_GL_OP_PARENS(buffer);

    static void deleter(const GLuint& id) { GL_CHECK(glDeleteBuffers(1, &id)); }
    inline bool valid() const { return buffer->valid(); }

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
        GL_CHECK(glGenBuffers(1, &buffer->id));
        set(target, usage);
    }

    // allows for binding buffer as alternative target
    inline void bindAs(GLenum _target) const {
        bind_helper(_target, buffer->id);
    }

    // call to bind the buffer to its target
    inline void bind() const {
        bindAs(target);
    }
    inline void unbind() {
        bind_helper(target, 0);
    }

    // binds to an index, which resizes when the buffer is resized
    inline void bindBase(GLuint index) const {
        GL_CHECK(glBindBufferBase(target, index, buffer->id));
    }
    inline void unbindBase(GLuint index) const {
        GL_CHECK(glBindBufferBase(target, index, 0));
    }

    // binds to an index with an optional offset, must be manually resized
    inline void bindRange(GLuint index, GLintptr offset = 0) const {
        assert(size > 0); // can't bind a range of length 0
        GL_CHECK(glBindBufferRange(target, index, buffer->id, offset, size));
    }
    inline void unbindRange(GLuint index) const {
        unbindBase(index); // equivalent operation, provided for convenience
    }

    // invalidates the buffer; used for streaming
    inline void invalidate() const {
        GL_CHECK(glBufferData(target, size, nullptr, usage));
        //GL_CHECK(glInvalidateBufferData(buffer->id));
    }

    // allocate the buffer after binding to contain [size] bytes from [_data].
    // IMPORTANT: the size of the array [_data] points to must match [size], or there will be access exceptions
    // calling these methods from an unbound buffer is allowed, but will have undefined results
    inline void data(const size_t size, const GLvoid* _data) {
        GL_CHECK(glBufferData(target, this->size = size, _data, usage));
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

    inline void storage(const size_t size, const GLvoid* data, const GLbitfield flags) {
        GL_CHECK(glBufferStorage(target, this->size = size, data, flags));
    }

private:
    static void bind_helper(GLenum target, GLint val) {
        switch (target) {
        case GL_ARRAY_BUFFER:
            GLstate<GL_BINDING, GL_ARRAY_BUFFER>{ val }.apply();
            break;
        case GL_ELEMENT_ARRAY_BUFFER:
            GLstate<GL_BINDING, GL_ELEMENT_ARRAY_BUFFER>{ val }.apply();
            break;
        default:
            GL_CHECK(glBindBuffer(target, val));
        }
    }
};

// wraps a VAO, stores bindings for attributes and buffers after binding
struct GLVAO {
    using handle_t = GLhandle<GLVAO>;
    shared<handle_t> vao = make_shared<handle_t>();
    inline WR_GL_OP_PARENS(vao);

    static void deleter(const GLuint& id) { GL_CHECK(glDeleteVertexArrays(1, &id)); }
    inline bool valid() const { return vao->valid(); }

    inline void create() const {
        if (valid()) return;
        GL_CHECK(glGenVertexArrays(1, &vao->id));
    }
    inline void bind() const {
        bind_helper(vao->id);
    }
    static inline void unbind() {
        bind_helper(0);
    }
private:
    static void bind_helper(GLint val) {
        GLstate<GL_BINDING, GL_VERTEX_ARRAY>{ val }.apply();
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

    enum class Type : GLenum {
        Vertex      = GL_VERTEX_SHADER,
        TessControl = GL_TESS_CONTROL_SHADER,
        TessEval    = GL_TESS_EVALUATION_SHADER,
        Geometry    = GL_GEOMETRY_SHADER,
        Fragment    = GL_FRAGMENT_SHADER,
        Compute     = GL_COMPUTE_SHADER
    };

    using handle_t = GLhandle<GLshader>;
    Type type;
    inline WR_GL_OP_PARENS_CONST(shader);
    inline WR_GL_OP_EQEQ(GLshader, shader);

    static void deleter(const GLuint& id) { GL_CHECK(glDeleteShader(id)); }
    inline bool valid() const { return shader->valid(); }

    // creates and compiles a shader of [type] from [body] and stores it
    inline void create(const char* body, Type type) {
        if (valid()) return;
        this->type = type;
        GL_CHECK(shader->id = glCreateShader((GLenum)type));
        GL_CHECK(glShaderSource(shader->id, 1, &body, 0));
        GL_CHECK(glCompileShader(shader->id));
    }

    inline GLint getVal(const GLenum value) const {
        GLint res;
        GL_CHECK(glGetShaderiv(shader->id, value, &res));
        return res;
    }
private:
    shared<handle_t> shader = make_shared<handle_t>();
    friend struct GLprogram;
};

// wraps a shader program [linked from several shaders]
struct GLprogram {
    GLshader vertex, tessControl, tessEval, geometry, fragment;
    GLshader compute; // this must be by itself
    bool isCompute = false;

    using handle_t = GLhandle<GLprogram>;
    shared<handle_t> program = make_shared<handle_t>();
    inline WR_GL_OP_PARENS(program);

    static void deleter(const GLuint& id) { GL_CHECK(glDeleteProgram(id)); }
    inline bool valid() const { return program->valid(); }

    inline const GLshader& getShaderByType(GLshader::Type type) {
        switch (type) {
        case GLshader::Type::Vertex:
            return vertex;
        case GLshader::Type::TessControl:
            return tessControl;
        case GLshader::Type::TessEval:
            return tessEval;
        case GLshader::Type::Geometry:
            return geometry;
        case GLshader::Type::Fragment:
            return fragment;
        case GLshader::Type::Compute:
            return compute;
        }
    }

    // deletes the program if it exists and creates a new one; this preserves the shaders used without resetting the pointer
    inline void refresh() {
        program->~GLhandle();
        program->id = def;
        create();
    }

    inline void create() {
        if (valid()) return;
        GL_CHECK(program->id = glCreateProgram());
    }

    inline GLint getVal(const GLenum value) const {
        GLint res;
        GL_CHECK(glGetProgramiv(program->id, value, &res));
        return res;
    }

    inline void attach(const GLshader& shader) const { GL_CHECK(glAttachShader(program->id, shader())); }

    inline void relink() const { GL_CHECK(glLinkProgram(program->id)); }

    // properly sets up the program once the shaders are set
    inline void link() {
        if (vertex.valid()) {
            attach(vertex);
            isCompute = false;

            if (tessControl.valid()) attach(tessControl);
            if (tessEval.valid())    attach(tessEval);
            if (geometry.valid())    attach(geometry);
            if (fragment.valid())    attach(fragment);
        }
        else if (compute.valid()) {
            attach(compute);
            isCompute = true;
        }
        relink();
    }

    inline void use() const {
        GLstate<GL_BINDING, GL_PROGRAM>{ (GLint)program->id }.apply();
    }

    inline void dispatch(GLuint xGroups = 1, GLuint yGroups = 1, GLuint zGroups = 1) const {
        assert(isCompute); // only compute shaders may be dispatched
        assert(xGroups < MAX_COMPUTE_WORK_GROUPS.x && yGroups < MAX_COMPUTE_WORK_GROUPS.y && zGroups < MAX_COMPUTE_WORK_GROUPS.z);
        GL_CHECK(glDispatchCompute(xGroups, yGroups, zGroups));
    }

    inline GLuint getUniformLocation(const char* name) const {
        GLuint l;
        GL_CHECK(l = glGetUniformLocation(program->id, name));
        return l;
    }

    // used for retrieving uniform variables (of type T)
    template<typename T>
    inline GLuniform<T> getUniform(const char* name) const {
        GLuniform<T> u;
        u.location = getUniformLocation(name);
        return u;
    }

    // binds a uniform block index to a slot index (must be called before use)
    inline void bindUniformBlock(const uint32_t location, const uint32_t index) const {
        GL_CHECK(glUniformBlockBinding(program->id, location, index));
    }

    // used for retrieving uniform block variables (of type T)
    template<typename T>
    inline GLuniformblock<T> getUniformBlock(const char* name) const {
        GLuniformblock<T> u;
        GL_CHECK(u.location = glGetUniformBlockIndex(program->id, name));
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
        GL_CHECK(index = glGetProgramResourceIndex(program->id, GL_UNIFORM, name));

        GLenum prop = GL_OFFSET;
        GLint length, param;
        GL_CHECK(glGetProgramResourceiv(program->id, GL_UNIFORM, index, 1, &prop, sizeof(GLuint), &length, &param));
        return param;
    }

    private:
        struct uivec3 { GLuint x, y, z; };
        static uivec3 MAX_COMPUTE_WORK_GROUPS;
        static void setMaxWorkGroups() {
            GL_CHECK(glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, (GLint*) &MAX_COMPUTE_WORK_GROUPS.x));
            GL_CHECK(glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, (GLint*) &MAX_COMPUTE_WORK_GROUPS.y));
            GL_CHECK(glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, (GLint*) &MAX_COMPUTE_WORK_GROUPS.z));
        }
        friend struct GLEWmanager;
};

// allows access to frame buffers
// For a frame buffer to be valid, it must meet the following requirements:
//   - Attached to at least one buffer (color, depth, stencil, etc.)
//   - Attached to at least one _color_
//   - All attachments are complete
//   - All attachments have the same number of multi-samples
struct GLframebuffer {
    using handle_t = GLhandle<GLframebuffer>;
    shared<handle_t> framebuffer = make_shared<handle_t>();
    GLenum type = GL_DRAW_FRAMEBUFFER;
    inline WR_GL_OP_PARENS(framebuffer);

    static void deleter(const GLuint& id) { GL_CHECK(glDeleteFramebuffers(1, &id)); }
    inline bool valid() const { return framebuffer->valid(); }

    inline GLenum check() const { GLenum res; GL_CHECK(res = glCheckFramebufferStatus(type)); return res; }
    inline bool checkComplete() const { return check() == GL_FRAMEBUFFER_COMPLETE; }
    inline bool isBound() const { return framebuffer->id == get_bound(type); }
    inline size_t numOutputs() const { return colorBuffers.size(); }

    static inline vec4 getClearColor() { return GLstate<GL_COLOR_CLEAR_VALUE>{}.state.color; }
    static inline void setClearColor(GLclampf r, GLclampf g, GLclampf b, GLclampf a) { GLstate<GL_COLOR_CLEAR_VALUE>{ vec4(r, g, b, a) }.apply(); }

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
        GL_CHECK(glGenFramebuffers(1, &framebuffer->id));
        type = target;
    }

    inline void bindPartial() const {
        bind_helper(type, framebuffer->id);
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
            GL_CHECK(glFramebufferTexture2D(type, attachment + colorBuffers.size(), tex.target, tex(), 0));
            colorBuffers.push_back(attachment + colorBuffers.size());
        }
        else
            GL_CHECK(glFramebufferTexture2D(type, attachment, tex.target, tex(), 0));
    }

    void rebindTexture(const GLtexture& tex, Attachment attachment, const uint32_t index = 0) {
        assert(attachment == Attachment::Color || index == 0); // everything except Color can't have multiple attachments
        assert(index <= colorBuffers.size()); // can't rebind what hasn't been bound

        GL_CHECK(glFramebufferTexture2D(type, attachment + index, tex.target, tex(), 0));
    }

    void unbind() const { unbind(type); }
    static void unbind(GLenum type) { bind_helper(type, 0); }

    // reads [width] x [height] pixels from the frame buffer starting from [x],[y] (lower-left corner) into [dest]
    // relevant to this function: GL_PIXEL_PACK_BUFFER and glPixelStore
    // in addition to the traditional color formats, [format] can accept GL_DEPTH_COMPONENT, GL_STENCIL_INDEX, GL_DEPTH_STENCIL to read those buffers
    // in the case of GL_STENCIL_INDEX, if the [type] is not GL_FLOAT, there will be masking: https://www.opengl.org/wiki/GLAPI/glReadPixels#Description
    void readPixels(void* dest, const int x, const int y, const size_t width, const size_t height, const GLenum texFormat = GL_RGBA, const GLenum unpackType = GLtype<uint32_t>()) const {
        assert(type == GL_FRAMEBUFFER || type == GL_READ_FRAMEBUFFER); // the frame buffer must be bound to GL_READ_FRAMEBUFFER (or GL_FRAMEBUFFER) for this operation to succeed
        assert(texFormat != GL_DEPTH_STENCIL || unpackType == GL_UNSIGNED_INT_24_8 || unpackType == GL_FLOAT_32_UNSIGNED_INT_24_8_REV); // GL_DEPTH_STENCIL has special requirements
        GL_CHECK(glReadPixels(x, y, width, height, texFormat, unpackType, dest));
    }

private:
    std::vector<GLenum> colorBuffers;

    static void bind_helper(GLenum target, GLint val) {
        switch (target) {
        case GL_DRAW_FRAMEBUFFER:
            GLstate<GL_BINDING, GL_DRAW_FRAMEBUFFER>{ val }.apply();
            break;
        case GL_READ_FRAMEBUFFER:
            GLstate<GL_BINDING, GL_READ_FRAMEBUFFER>{ val }.apply();
            break;
        case GL_FRAMEBUFFER:
            GLstate<GL_BINDING, GL_DRAW_FRAMEBUFFER>{ val }.apply();
            GLstate<GL_BINDING, GL_READ_FRAMEBUFFER>{ val }.apply();
            break;
        default:
            GL_CHECK(glBindFramebuffer(target, val));
        }
    }

    static GLuint get_bound(GLenum target) {
        switch (target) {
        case GL_DRAW_FRAMEBUFFER:
            return GLstate<GL_BINDING, GL_DRAW_FRAMEBUFFER>{}.state.bound;
        case GL_READ_FRAMEBUFFER:
            return GLstate<GL_BINDING, GL_READ_FRAMEBUFFER>{}.state.bound;
        case GL_FRAMEBUFFER:
        {
            auto drawBound = GLstate<GL_BINDING, GL_DRAW_FRAMEBUFFER>{}.state.bound;
            assert((drawBound == GLstate<GL_BINDING, GL_READ_FRAMEBUFFER>{}.state.bound));
            return drawBound;
        }
        default:
            return def;
        }
    }

    // if errors persist, GL_MAX_DRAW_BUFFERS is how many buffers can be drawn to per draw call
    static GLint MAX_COLOR_ATTACHMENTS;
    static void setMaxColorAttachments() {
        MAX_COLOR_ATTACHMENTS = local(getMaxColorAttachments)();
    }
    friend struct GLEWmanager;
};

struct GLquery {
    using handle_t = GLhandle<GLquery>;
    shared<handle_t> query = make_shared<handle_t>();

    enum Target : GLenum {
        SamplesPassed                      = GL_SAMPLES_PASSED, // the samples passed queries refer to how many samples passed the depth test
        AnySamplesPassed                   = GL_ANY_SAMPLES_PASSED,
        AnySamplesPassedFast               = GL_ANY_SAMPLES_PASSED_CONSERVATIVE, // the original name is dumb/inaccurate
        PrimitivesGenerated                = GL_PRIMITIVES_GENERATED,
        TransformFeedbackPrimitivesWritten = GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN,
        TimeElapsed                        = GL_TIME_ELAPSED,
        Timestamp                          = GL_TIMESTAMP
    };

    Target target;
    inline WR_GL_OP_PARENS(query);

    static void deleter(const GLuint& id) { GL_CHECK(glDeleteQueries(1, &id)); }
    inline bool valid() const { return query->valid(); }

    inline void create(Target target) {
        GL_CHECK(glGenQueries(1, &query->id));
        this->target = target;
    }

    inline void execute() { 
        assert(state != IN_PROGRESS);
        if (target == Target::Timestamp) {
            GL_CHECK(glQueryCounter(target, query->id));
            state = DONE;
        }
        else {
            GL_CHECK(glBeginQuery(target, query->id));
            state = IN_PROGRESS;
        }
    }
    inline void finish() { 
        assert(state == IN_PROGRESS);
        GL_CHECK(glEndQuery(target));
        state = DONE;
    }

    // synchronous with GPU; TODO make async version with callback
    int64_t getResult() {
        assert(state == DONE);
        int64_t val;
        GL_CHECK(glGetQueryObjecti64v(target, GL_QUERY_RESULT, &val));
        return val;
    }
private:
    enum { PENDING, IN_PROGRESS, DONE } state = PENDING;
};

namespace GLsynchro {
    static inline void barrier(GLbitfield barrier)         { GL_CHECK(glMemoryBarrier(barrier)); }
    static inline void barrierByRegion(GLbitfield barrier) { GL_CHECK(glMemoryBarrierByRegion(barrier)); }
};

#include "GLresource.h"

// helper class, used to assist in creating vertex array attribute bindings (create and use locally, DO NOT STORE!)
class GLattrarr {
    struct GLattr {
        GLenum type; GLuint size, bytes, divisor; bool normalize, castToFloat;
    };
    inline void reset() {
        attrs.clear();
    }

    static GLint MAX_VERTEX_ATTRIBS;
    static void setMaxColorAttachments() {
        MAX_VERTEX_ATTRIBS = local(getMaxVertexAttribs)();
    }
    friend struct GLEWmanager;
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
            assert(i + baseIndex < (size_t)MAX_VERTEX_ATTRIBS); // the index exceeded the max available attributes
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
};
