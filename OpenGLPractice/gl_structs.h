#pragma once

#include "GL/glew.h"

#include <vector>

#include "MarchMath.h"
#include "smart_ptr.h"

#define local(name) __local__ ## name

typedef GLint GLsampler;

namespace {
	GLuint def = (GLuint) -1;

	void local(delBuffer)(GLuint* b) { if (*b != def) glDeleteBuffers(1, b); delete b; }
	void local(delVAO)(GLuint* a) { if (*a != def) glDeleteVertexArrays(1, a); delete a; }
		 
	void local(delShader)(GLuint* s) { if (*s == def) glDeleteShader(*s); delete s; }
	void local(delShaderProg)(GLuint* p) { if (*p != def) glDeleteProgram(*p); delete p; }
}

template<typename T> inline GLenum GLtype() { return GL_FALSE; }
template<> inline GLenum GLtype<GLbyte>()  { return GL_BYTE;  } template<> inline GLenum GLtype<GLubyte>()  { return GL_UNSIGNED_BYTE; }
template<> inline GLenum GLtype<GLshort>() { return GL_SHORT; } template<> inline GLenum GLtype<GLushort>() { return GL_UNSIGNED_SHORT; }
template<> inline GLenum GLtype<GLint>()   { return GL_INT;   } template<> inline GLenum GLtype<GLuint>()   { return GL_UNSIGNED_INT; }
template<> inline GLenum GLtype<GLfloat>() { return GL_FLOAT; } template<> inline GLenum GLtype<GLdouble>() { return GL_DOUBLE; }

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

struct GLbuffer {
	unique<GLuint, void(*)(GLuint*)> buffer{ new GLuint(def), local(delBuffer) };// unique because sharing could cause conflicts
	GLenum target, usage;
	size_t size;
	GLuint& operator()() { return *buffer; }
	inline void set(GLenum target, GLenum usage) { this->target = target; this->usage = usage; }

	inline void create(GLenum target, GLenum usage = GL_STATIC_DRAW) { glGenBuffers(1, buffer.get()); set(target, usage); }
	inline void bind() { glBindBuffer(target, *buffer); }
	// calling these methods from an unbound buffer is allowed, but will have undefined results
	inline void data(size_t size, const void* _data) { glBufferData(target, size, _data, usage); this->size = size; }
	// this version is intended for updates
	inline void data(const void* _data) { 
		if (usage == GL_STATIC_DRAW || !size) return; 
		glBufferData(target, size, nullptr, usage);
		glBufferData(target, size, _data, usage);
	}
};

struct GLVAO {
	shared<GLuint> vao{ new GLuint(def), local(delVAO) };
	GLuint& operator()() { return *vao; }

	inline void create() { glGenVertexArrays(1, vao.get()); }
	inline void bind() { glBindVertexArray(*vao); }
};

struct GLshader {
	GLenum type = def;
	GLuint& operator()() { return *shader; }
	inline bool valid() { return type != def; }

	void create(const char* fileContents, GLenum type) { 
		this->type = type;
		*shader = glCreateShader(type);
		glShaderSource(*shader, 1, &fileContents, 0);
		glCompileShader(*shader);
	}
private:
	shared<GLuint> shader{ new GLuint(def), local(delShader) };
	friend struct GLprogram;
};

struct GLprogram {
	GLshader vertex, fragment;// this can be extended when necessary for geometry and tesselation shaders, or use a std::vector
	shared<GLuint> program{ new GLuint(def), local(delShaderProg) };
	GLuint& operator()() { return *program; }

	void create() { *program = glCreateProgram(); }
	void link() { 
		if (vertex.valid())   glAttachShader(*program, vertex());
		if (fragment.valid()) glAttachShader(*program, fragment());
		glLinkProgram(*program);
	}

	inline void use() { glUseProgram(*program); }
	template<typename T> inline GLuniform<T> getUniform(const char* name) { GLuniform<T> u; u.location = glGetUniformLocation(*program, name); return u; }
};

// meant for use in binding setup, store locally
class GLattrarr {
	struct GLattr { GLenum type; GLuint size, bytes, divisor; bool normalize; };
	void reset() { attrs = std::vector<GLattr>(); }

public:
	std::vector<GLattr> attrs;

	template<typename T>
	// type is the enum value of T
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
			add<GLfloat>(3, 1, normalize);
			add<GLfloat>(3, 1, normalize);
			add<GLfloat>(3, 1, normalize);
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