#pragma once

#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include <cstdlib>

struct GLFWmanager {
    static bool initialized;
    ~GLFWmanager() {
        if (initialized) {
            glfwTerminate();
            initialized = false;
        }
    }
    GLFWmanager(const size_t width, const size_t height);
};

struct GLEWmanager {
    static bool initialized;
    GLEWmanager() {
        
        if (initialized) return;

        glewExperimental = GL_TRUE;
        auto initValue = glewInit();
        initialized = initValue == GLEW_OK;
        if (!initialized) exit(initValue);
    }
};