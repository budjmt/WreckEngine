#pragma once

#include "GL/glew.h"

#include <vector>

#include "MarchMath.h"
#include "smart_ptr.h"

#define local(name) __local__ ## name

typedef GLint GLsampler;

namespace {
	// default value used to represent "unintialized" resources
	const GLuint def = (GLuint) -1;

	size_t local(getMaxNumTextures)() { GLint val; glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &val); return val; }

	void local(delTexture)    (GLuint* t) { if (*t != def) glDeleteTextures(1, t);    delete t; }

	void local(delBuffer)    (GLuint* b) { if (*b != def) glDeleteBuffers(1, b);      delete b; }
	void local(delVAO)       (GLuint* a) { if (*a != def) glDeleteVertexArrays(1, a); delete a; }
		 
	void local(delShader)    (GLuint* s) { if (*s != def) glDeleteShader(*s);  delete s; }
	void local(delShaderProg)(GLuint* p) { if (*p != def) glDeleteProgram(*p); delete p; }
}

// maps primitive types to their matching GLenum values, (if applicable) or returns GL_FALSE.
template<typename T> inline GLenum GLtype() { return GL_FALSE; }
template<> inline GLenum GLtype<GLbyte>()  { return GL_BYTE;  } template<> inline GLenum GLtype<GLubyte>()  { return GL_UNSIGNED_BYTE; }
template<> inline GLenum GLtype<GLshort>() { return GL_SHORT; } template<> inline GLenum GLtype<GLushort>() { return GL_UNSIGNED_SHORT; }
template<> inline GLenum GLtype<GLint>()   { return GL_INT;   } template<> inline GLenum GLtype<GLuint>()   { return GL_UNSIGNED_INT; }
template<> inline GLenum GLtype<GLfloat>() { return GL_FLOAT; } template<> inline GLenum GLtype<GLdouble>() { return GL_DOUBLE; }

// wraps a location pointing to a uniform variable of type T. the value is updated using update(T t). If there is no definition for update, the type is unsupported.
template<typename T> struct GLuniform { GLuint location; };
template<> struct GLuniform<GLint>    { GLuint location; void update(GLint value)    const { glUniform1i(location, value);  }; };
template<> struct GLuniform<GLuint>   { GLuint location; void update(GLuint value)   const { glUniform1ui(location, value); }; };
template<> struct GLuniform<GLfloat>  { GLuint location; void update(GLfloat value)  const { glUniform1f(location, value);  }; };
template<> struct GLuniform<GLdouble> { GLuint location; void update(GLdouble value) const { glUniform1d(location, value);  }; };
template<> struct GLuniform<vec2> { GLuint location; void update(const vec2& value) const { glUniform2fv(location, 1, &value[0]); }; };
template<> struct GLuniform<vec3> { GLuint location; void update(const vec3& value) const { glUniform3fv(location, 1, &value[0]); }; };
template<> struct GLuniform<vec4> { GLuint location; void update(const vec4& value) const { glUniform4fv(location, 1, &value[0]); }; };
template<> struct GLuniform<mat3> { GLuint location; void update(const mat3& value, bool transpose = false) const { glUniformMatrix3fv(location, 1, transpose, &value[0][0]); }; };
template<> struct GLuniform<mat4> { GLuint location; void update(const mat4& value, bool transpose = false) const { glUniformMatrix4fv(location, 1, transpose, &value[0][0]); }; };

// wraps a texture. [type] reflects what type of sampler it needs.
// https://www.opengl.org/wiki/Sampler_(GLSL)
struct GLtexture {
	static const GLint MAX_TEXTURES;
	shared<GLuint> texture = shared<GLuint>(new GLuint(def), local(delTexture));
	GLenum type;
	GLuint& operator()() { return *texture; }

	void create(GLenum type = GL_TEXTURE_2D) { glGenTextures(1, texture.get()); this->type = type; }
	void bind(GLint index = 0) { 
		if (index > MAX_TEXTURES) return; 
		glActiveTexture(GL_TEXTURE0 + index); 
		glBindTexture(type, *texture);
	}

	// these sets correspond to glTextImage. Texture must be bound for these to work.
	template<typename value_T>
	void set1D(GLvoid* pixelData, GLuint width, GLenum format_from = GL_RGBA, GLenum format_to = GL_RGBA, GLint mip_level = 0) {
		glTexImage1D(type, mip_level, format_to, width, 0, format_from, GLtype<value_T>(), pixelData);
	}
	template<typename value_T>
	void set2D(GLvoid* pixelData, GLuint width, GLuint height, GLenum format_from = GL_RGBA, GLenum format_to = GL_RGBA, GLint mip_level = 0) {
		glTexImage2D(type, mip_level, format_to, width, height, 0, format_from, GLtype<value_T>(), pixelData);
	}
	template<typename value_T>
	void set3D(GLvoid* pixelData, GLuint width, GLuint height, GLuint depth, GLenum format_from = GL_RGBA, GLenum format_to = GL_RGBA, GLint mip_level = 0) {
		glTexImage3D(type, mip_level, format_to, width, height, depth, 0, format_from, GLtype<value_T>(), pixelData);
	}
};

// wraps a buffer object on the GPU.
struct GLbuffer {
	unique<GLuint, void(*)(GLuint*)> buffer{ new GLuint(def), local(delBuffer) };// unique because sharing could cause conflicts
	GLenum target, usage;
	size_t size;
	GLuint& operator()() { return *buffer; }
	inline void set(GLenum target, GLenum usage) { this->target = target; this->usage = usage; }

	// call this to replace the currently stored buffer index with a newly created one.
	// target: GL_ARRAY_BUFFER or GL_ELEMENT_ARRAY_BUFFER
	// usage: GL_STATIC_DRAW by default, also can be GL_STREAM_DRAW or GL_DYNAMIC_DRAW
	inline void create(GLenum target, GLenum usage = GL_STATIC_DRAW) { if (*buffer != def) return; glGenBuffers(1, buffer.get()); set(target, usage); }
	
	// call to bind the buffer to its target
	inline void bind() { glBindBuffer(target, *buffer); }

	// allocate the buffer after binding to contain [size] bytes from [_data].
	// IMPORTANT: the size of the array [_data] points to must match [size], or there will be access exceptions
	// calling these methods from an unbound buffer is allowed, but will have undefined results
	inline void data(size_t size, const void* _data) { glBufferData(target, size, _data, usage); this->size = size; }
	
	// this version is intended for updates, not instantiations
	inline void data(const void* _data) { 
		if (usage == GL_STATIC_DRAW || !size) return; 
		glBufferData(target, size, nullptr, usage);
		glBufferData(target, size, _data, usage);
	}
};

// wraps a VAO, stores bindings for attributes and buffers after binding
struct GLVAO {
	shared<GLuint> vao{ new GLuint(def), local(delVAO) };
	GLuint& operator()() { return *vao; }

	inline void create() { if (*vao != def) return; glGenVertexArrays(1, vao.get()); }
	inline void bind() { glBindVertexArray(*vao); }
};

// wraps a compiled shader
struct GLshader {
	GLenum type = def;
	GLuint& operator()() { return *shader; }
	inline bool valid() { return type != def; }

	// creates and compiles a shader of [type] from [body] and stores it
	void create(const char* body, GLenum type) { 
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
	GLshader vertex, fragment;// this can be extended when necessary for geometry and tesselation shaders, or use a std::vector
	shared<GLuint> program{ new GLuint(def), local(delShaderProg) };
	GLuint& operator()() { return *program; }

	void create() { if (*program != def) return; *program = glCreateProgram(); }
	// properly sets up the program once the shaders are set
	void link() { 
		if (vertex.valid())   glAttachShader(*program, vertex());
		if (fragment.valid()) glAttachShader(*program, fragment());
		glLinkProgram(*program);
	}

	inline void use() { glUseProgram(*program); }
	// used for retrieving uniform variables (of type T)
	template<typename T> inline GLuniform<T> getUniform(const char* name) { GLuniform<T> u; u.location = glGetUniformLocation(*program, name); return u; }
};

// helper class, used to assist in creating vertex array attribute bindings (create and use locally, DO NOT STORE!)
class GLattrarr {
	struct GLattr { GLenum type; GLuint size, bytes, divisor; bool normalize; };
	void reset() { attrs = std::vector<GLattr>(); }

public:
	std::vector<GLattr> attrs;

	// adds an attritube of type T to the cache. Use a divisor value of 1 for instanced variables
	template<typename T>
	void add(size_t size, GLuint divisor = 0, bool normalize = GL_FALSE) {
		GLattr attr;
		attr.type = GLtype<T>();
		attr.size = size;
		attr.divisor = divisor;
		attr.bytes = sizeof(T) * size;
		attr.normalize = normalize;

		attrs.push_back(attr);
	}

	template<> void add<vec3>(size_t size, GLuint divisor, bool normalize) { for (; size; --size) add<GLfloat>(3, divisor, normalize); }
	template<> void add<vec4>(size_t size, GLuint divisor, bool normalize) { for (; size; --size) add<GLfloat>(4, divisor, normalize); }
	template<> void add<mat3>(size_t size, GLuint divisor, bool normalize) {
		for (; size; --size) {
			add<GLfloat>(3, divisor, normalize);
			add<GLfloat>(3, divisor, normalize);
			add<GLfloat>(3, divisor, normalize);
		}
	}
	template<> void add<mat4>(size_t size, GLuint divisor, bool normalize) {
		for (; size; --size) {
			add<GLfloat>(4, divisor, normalize);
			add<GLfloat>(4, divisor, normalize);
			add<GLfloat>(4, divisor, normalize);
			add<GLfloat>(4, divisor, normalize);
		}
	}

	// call once all desired attributes have been added; applies the added attributes, starting at [baseIndex] and clears the cache.
	// [baseIndex] would be used when there are multiple buffers bound to a single shader, e.g. instanced rendering, where the stride must be reset
	void apply(GLuint baseIndex = 0) {
		size_t stride = 0;
		for (auto& attr : attrs) { stride += attr.bytes; }
		for (size_t i = 0, offset = 0, entries = attrs.size(); i < entries; ++i) {
			auto& attr = attrs[i];
			glEnableVertexAttribArray(i + baseIndex);
			glVertexAttribPointer(i + baseIndex, attr.size, attr.type, attr.normalize, stride, (void*)offset);
			if (attr.divisor) glVertexAttribDivisor(i + baseIndex, attr.divisor);
			offset += attr.bytes;
		}
		reset();
	}
};