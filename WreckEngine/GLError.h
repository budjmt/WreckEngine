#ifndef P_GL_ERROR
#define P_GL_ERROR

#include "GL/glew.h"
#include "glfw/glfw3.h"

#include <stdio.h>

#if defined(_MSC_VER)
#define WR_CURRENT_FUNCTION __FUNCSIG__
#else
#define WR_CURRENT_FUNCTION __PRETTY_FUNCTION__
#endif

#define BREAK "----------------------------------------------\n"

#define STRINGIZE2(x) #x
#define STRINGIZE(x) STRINGIZE2(x)

#define CLEAR_GL_ERR glGetError()
#define CHECK_GL_ERR GLCheckError("GL-ERROR; \"" __FILE__ "\" before line " STRINGIZE(__LINE__) ": \n")

#if defined(_DEBUG) && !defined(NDEBUG)
#define GL_CHECK(x) { x; ::CheckGlErrorImpl(#x, __FILE__, __LINE__, WR_CURRENT_FUNCTION); }
#else
#define GL_CHECK(x) x
#endif

inline const char* const GetGlErrorString(GLenum error) {
    switch (error) {
        case GL_NO_ERROR:
            return "No error has been recorded.";
        case GL_INVALID_ENUM:
            return "An unacceptable value is specified for an enumerated argument.";
        case GL_INVALID_VALUE:
            return "A numeric argument is out of range.";
        case GL_INVALID_OPERATION:
            return "The specified operation is not allowed in the current state.";
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            return "The framebuffer object is not complete.";
        case GL_OUT_OF_MEMORY:
            return "There is not enough memory left to execute the command.";
        case GL_STACK_UNDERFLOW:
            return "An attempt has been made to perform an operation that would cause an internal stack to underflow.";
        case GL_STACK_OVERFLOW:
            return "An attempt has been made to perform an operation that would cause an internal stack to overflow.";
        default:
            return "An unknown error has occurred.";
    }
}

inline bool CheckGlErrorImpl(const char* call, const char* file, int line, const char* function) {
    //GLenum error = glGetError();
    //if (error == GL_NO_ERROR) return false;
    //
    //printf("[GL] '%s': %s\n  FILE: %s(%i)\n  FUNC: %s\n\n", call, GetGlErrorString(error), file, line, function);
    return true;
}

static void GLPrintError(GLenum error, const char* prepend = "") {
    printf(BREAK "%s %s\n" BREAK, prepend, GetGlErrorString(error));
}

static inline void GLCheckError(const char* prepend = "") {
        auto err = glGetError();
        if (err) GLPrintError(err, prepend);
}

#endif