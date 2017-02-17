#pragma once

#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include <cstdlib>

struct GLFWmanager {
    static bool initialized;
    ~GLFWmanager() {
        if (initialized) {
            initialized = false;
            glfwTerminate();
        }
    }
    GLFWmanager(const size_t width, const size_t height);

    static GLFWmonitor* getMonitor() { return glfwGetPrimaryMonitor(); }
};

struct GLEWmanager {
    static bool initialized;
    GLEWmanager() {
        
        if (initialized) return;

        glewExperimental = GL_TRUE;
        auto initValue = glewInit();
        initialized = initValue == GLEW_OK;
        if (!initialized) exit(initValue);

        initGLValues();
    }

    void initGLValues();
};