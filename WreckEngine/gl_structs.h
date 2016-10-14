#pragma once

#include "GL/glew.h"

#include "GLError.h"

#include <vector>

#include "MarchMath.h"
#include "smart_ptr.h"

#define local(name) __local__ ## name

typedef GLint GLsampler;

namespace {
	// default value used to represent "uninitialized" resources
	constexpr GLuint def = (GLuint) -1;

	GLint local(getMaxNumTextures)() { GLint val; glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &val); return val; }

	void local(delTexture)   (GLuint* t) { if (*t != def) glDeleteTextures(1, t);     delete t; }

	void local(delBuffer)    (GLuint* b) { if (*b != def) glDeleteBuffers(1, b);      delete b; }
	void local(delVAO)       (GLuint* a) { if (*a != def) glDeleteVertexArrays(1, a); delete a; }
		 
	void local(delShader)    (GLuint* s) { if (*s != def) glDeleteShader(*s);  delete s; }
	void local(delShaderProg)(GLuint* p) { if (*p != def) glDeleteProgram(*p); delete p; }
}

struct GLtexture;
struct GLbuffer; 
struct GLVAO; 
struct GLshader;
struct GLprogram;

// maps primitive types to their matching GLenum values, (if applicable) or returns GL_FALSE.
template<typename T> inline constexpr GLenum GLtype() { return GL_FALSE; }
template<> inline constexpr GLenum GLtype<GLbyte>()  { return GL_BYTE;  } template<> inline constexpr GLenum GLtype<GLubyte>()  { return GL_UNSIGNED_BYTE; }
template<> inline constexpr GLenum GLtype<GLshort>() { return GL_SHORT; } template<> inline constexpr GLenum GLtype<GLushort>() { return GL_UNSIGNED_SHORT; }
template<> inline constexpr GLenum GLtype<GLint>()   { return GL_INT;   } template<> inline constexpr GLenum GLtype<GLuint>()   { return GL_UNSIGNED_INT; }
template<> inline constexpr GLenum GLtype<GLfloat>() { return GL_FLOAT; } template<> inline constexpr GLenum GLtype<GLdouble>() { return GL_DOUBLE; }

template<> inline constexpr GLenum GLtype<GLtexture>() { return GL_TEXTURE; }
template<> inline constexpr GLenum GLtype<GLbuffer>()  { return GL_BUFFER;  }
template<> inline constexpr GLenum GLtype<GLVAO>()     { return GL_VERTEX_ARRAY; }
template<> inline constexpr GLenum GLtype<GLshader>()  { return GL_SHADER;  }
template<> inline constexpr GLenum GLtype<GLprogram>() { return GL_PROGRAM; }

// wraps a location pointing to a uniform variable of type T. the value is updated using update(T t). If there is no definition for update, the type is unsupported.
template<typename T> struct GLuniform { GLuint location; };
template<> struct GLuniform<GLint>    { GLuint location; inline void update(GLint value)    const { glUniform1i(location, value);  }; };
template<> struct GLuniform<GLuint>   { GLuint location; inline void update(GLuint value)   const { glUniform1ui(location, value); }; };
template<> struct GLuniform<GLfloat>  { GLuint location; inline void update(GLfloat value)  const { glUniform1f(location, value);  }; };
template<> struct GLuniform<GLdouble> { GLuint location; inline void update(GLdouble value) const { glUniform1d(location, value);  }; };
template<> struct GLuniform<vec2> { GLuint location; inline void update(const vec2& value) const { glUniform2fv(location, 1, &value[0]); }; };
template<> struct GLuniform<vec3> { GLuint location; inline void update(const vec3& value) const { glUniform3fv(location, 1, &value[0]); }; };
template<> struct GLuniform<vec4> { GLuint location; inline void update(const vec4& value) const { glUniform4fv(location, 1, &value[0]); }; };
template<> struct GLuniform<mat3> { GLuint location; inline void update(const mat3& value, bool transpose = false) const { glUniformMatrix3fv(location, 1, transpose, &value[0][0]); }; };
template<> struct GLuniform<mat4> { GLuint location; inline void update(const mat4& value, bool transpose = false) const { glUniformMatrix4fv(location, 1, transpose, &value[0][0]); }; };

// wraps a texture. [type] reflects what type of sampler it needs.
// https://www.opengl.org/wiki/Sampler_(GLSL)
struct GLtexture {
	shared<GLuint> texture = shared<GLuint>(new GLuint(def), local(delTexture));
	GLenum type;
	inline GLuint& operator()() const { return *texture; }
	inline void create(const GLenum _type = GL_TEXTURE_2D, const GLint max_mip_level = 0) { 
		type = _type;
		glGenTextures(1, texture.get());
		// these are globally bound, so technically this line affects every texture [of the type] each time the value changes
		// this can be fixed with immutable textures in 4.3+
		param(GL_TEXTURE_BASE_LEVEL, 0);
		param(GL_TEXTURE_MAX_LEVEL, max_mip_level);
	}
	// must be done while bound
	inline void genMipMap() { glGenerateMipmap(type); }
	inline void bind(const GLint index = 0) const { 
		if (index > MAX_TEXTURES) return; 
		glActiveTexture(GL_TEXTURE0 + index); 
		glBindTexture(type, *texture);
	}
	inline void unbind() { glBindTexture(type, 0); }

	inline void param(const GLenum name, const int val) { glTexParameteri(type, name, val); }
	inline void param(const GLenum name, const float val) { glTexParameterf(type, name, val); }

	// these sets correspond to glTexImage. Texture must be bound for these to work.
	template<typename value_T>
	inline void set1D(const GLvoid* pixelData, const GLuint width, const GLenum format_from = GL_RGBA, const GLenum format_to = GL_RGBA, const GLint mip_level = 0) const {
		glTexImage1D(type, mip_level, format_to, width, 0, format_from, GLtype<value_T>(), pixelData);
	}
	template<typename value_T>
	inline void set2D(const GLvoid* pixelData, const GLuint width, const GLuint height, const GLenum format_from = GL_RGBA, const GLenum format_to = GL_RGBA, const GLint mip_level = 0) const {
		glTexImage2D(type, mip_level, format_to, width, height, 0, format_from, GLtype<value_T>(), pixelData);
	}
	template<typename value_T>
	inline void set3D(const GLvoid* pixelData, const GLuint width, const GLuint height, const GLuint depth, const GLenum format_from = GL_RGBA, const GLenum format_to = GL_RGBA, const GLint mip_level = 0) const {
		glTexImage3D(type, mip_level, format_to, width, height, depth, 0, format_from, GLtype<value_T>(), pixelData);
	}

private:
	static GLint MAX_TEXTURES;
	static void setMaxTextures() { MAX_TEXTURES = local(getMaxNumTextures)(); }
	friend struct GLFWmanager;
};

// wraps a buffer object on the GPU.
struct GLbuffer {
	unique<GLuint, void(*)(GLuint*)> buffer{ new GLuint(def), local(delBuffer) };// unique because sharing could cause conflicts
	GLenum target, usage;
	size_t size;
	inline GLuint& operator()() const { return *buffer; }
	inline void set(const GLenum target, const GLenum usage) { this->target = target; this->usage = usage; }

	// call this to replace the currently stored buffer index with a newly created one.
	// target: GL_ARRAY_BUFFER or GL_ELEMENT_ARRAY_BUFFER
	// usage: GL_STATIC_DRAW by default, also can be GL_STREAM_DRAW or GL_DYNAMIC_DRAW
	inline void create(const GLenum target, const GLenum usage = GL_STATIC_DRAW) { if (*buffer != def) return; glGenBuffers(1, buffer.get()); set(target, usage); }
	
	// call to bind the buffer to its target
	inline void bind() const { glBindBuffer(target, *buffer); }
	inline void unbind() { glBindBuffer(target, 0); }

	// allocate the buffer after binding to contain [size] bytes from [_data].
	// IMPORTANT: the size of the array [_data] points to must match [size], or there will be access exceptions
	// calling these methods from an unbound buffer is allowed, but will have undefined results
	inline void data(const size_t size, const GLvoid* _data) { glBufferData(target, size, _data, usage); this->size = size; }
	
	// this version is intended for updates, (for streams) not instantiations
	inline void data(const GLvoid* _data) const { 
		if (usage == GL_STATIC_DRAW || !size) return; 
		glBufferData(target, size, nullptr, usage);
		glBufferData(target, size, _data, usage);
	}

	inline void subdata(const GLvoid* _data, const GLuint _size, const GLuint offset = 0) const {
		if (usage == GL_STATIC_DRAW || !size) return;
		glBufferSubData(target, offset, _size, _data);
	}
};

// wraps a VAO, stores bindings for attributes and buffers after binding
struct GLVAO {
	shared<GLuint> vao{ new GLuint(def), local(delVAO) };
	inline GLuint& operator()() const { return *vao; }

	inline void create() const { if (*vao != def) return; glGenVertexArrays(1, vao.get()); }
	inline void bind() const { glBindVertexArray(*vao); }
	static inline void unbind() { glBindVertexArray(0); }
};

// wraps a compiled shader
struct GLshader {
	GLenum type = def;
	inline GLuint& operator()() const { return *shader; }
	inline bool valid() const { return type != def; }

	// creates and compiles a shader of [type] from [body] and stores it
	inline void create(const char* body, const GLenum type) { 
		if (valid()) return;
		this->type = type;
		*shader = glCreateShader(type);
		glShaderSource(*shader, 1, &body, 0);
		glCompileShader(*shader);
	}
private:
	shared<GLuint> shader{ new GLuint(def), local(delShader) };
	friend struct GLprogram;
};

// wraps a shader program [linked from several shaders]
struct GLprogram {
	GLshader vertex, fragment;// this can be extended when necessary for geometry and tessellation shaders, or use a std::vector
	shared<GLuint> program{ new GLuint(def), local(delShaderProg) };
	inline GLuint& operator()() const { return *program; }

	inline void create() { if (*program != def) return; *program = glCreateProgram(); }
	// properly sets up the program once the shaders are set
	inline void link() const { 
		if (vertex.valid())   glAttachShader(*program, vertex());
		if (fragment.valid()) glAttachShader(*program, fragment());
		glLinkProgram(*program);
	}

	inline void use() const { glUseProgram(*program); }
	// used for retrieving uniform variables (of type T)
	template<typename T> inline GLuniform<T> getUniform(const char* name) const { GLuniform<T> u; u.location = glGetUniformLocation(*program, name); return u; }
};

// helper class, used to assist in creating vertex array attribute bindings (create and use locally, DO NOT STORE!)
class GLattrarr {
	struct GLattr { GLenum type; GLuint size, bytes, divisor; bool normalize; };
	inline void reset() { attrs = std::vector<GLattr>(); }

public:
	std::vector<GLattr> attrs;

	// adds an attribute of type T to the cache. Use a divisor value of 1 for instanced variables
	template<typename T>
	inline void add(const size_t size, const GLuint divisor = 0, const bool normalize = GL_FALSE) {
		GLattr attr;
		attr.type = GLtype<T>();
		attr.size = size;
		attr.divisor = divisor;
		attr.bytes = sizeof(T) * size;
		attr.normalize = normalize;

		attrs.push_back(attr);
	}

	template<> inline void add<vec3>(const size_t size, const GLuint divisor, const bool normalize) { for (size_t i = 0; i < size; ++i) add<GLfloat>(3, divisor, normalize); }
	template<> inline void add<vec4>(const size_t size, const GLuint divisor, const bool normalize) { for (size_t i = 0; i < size; ++i) add<GLfloat>(4, divisor, normalize); }
	template<> inline void add<mat3>(const size_t size, const GLuint divisor, const bool normalize) {
		for (size_t i = 0; i < size; ++i) {
			add<GLfloat>(3, divisor, normalize);
			add<GLfloat>(3, divisor, normalize);
			add<GLfloat>(3, divisor, normalize);
		}
	}
	template<> inline void add<mat4>(const size_t size, const GLuint divisor, const bool normalize) {
		for (size_t i = 0; i < size; ++i) {
			add<GLfloat>(4, divisor, normalize);
			add<GLfloat>(4, divisor, normalize);
			add<GLfloat>(4, divisor, normalize);
			add<GLfloat>(4, divisor, normalize);
		}
	}

	// call once all desired attributes have been added; applies the added attributes, starting at [baseIndex] and clears the cache.
	// [baseIndex] would be used when there are multiple buffers bound to a single shader, e.g. instanced rendering, where the stride must be reset
	inline void apply(const GLuint baseIndex = 0) {
		size_t stride = 0;
		for (const auto& attr : attrs) { stride += attr.bytes; }
		for (size_t i = 0, offset = 0, entries = attrs.size(); i < entries; ++i) {
			const auto& attr = attrs[i];
			glEnableVertexAttribArray(i + baseIndex);
			glVertexAttribPointer(i + baseIndex, attr.size, attr.type, attr.normalize, stride, (void*)offset);
			if (attr.divisor) glVertexAttribDivisor(i + baseIndex, attr.divisor);
			offset += attr.bytes;
		}
		reset();
	}
};