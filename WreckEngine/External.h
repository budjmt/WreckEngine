#pragma once

#include "GLFW/glfw3.h"
#include "glm/glm.hpp"

#include "Random.h"

struct Window {
    static GLFWwindow* window;
    static int width, height;
    static int frameWidth, frameHeight;
    static float aspect;
    static int cursorMode;

    // These two are purely for convenience with ImGui
    static vec2 size;
    static vec2 frameScale;

    static inline int getKey(const int keyCode) { return glfwGetKey(window, keyCode); }

    static inline void resize_callback(GLFWwindowsizefun f) { glfwSetWindowSizeCallback(window, f); }

    static void default_resize(GLFWwindow* window, int width, int height);
};

struct Mouse {
    // this style enables copies to be made, for better concurrency
    struct Info {
        struct button { double lastDown = 0; bool downThisFrame = false; };
        struct cursor { double x = 0, y = 0; };

        uint32_t down = 0;// bit-field
        button buttons[3];
        cursor curr, currPixel, prev;
        const double clickCoolDown = 0.2;
        float wheel = 0.f;

        inline bool getButtonState(int b) { return buttons[b].downThisFrame || (down & (1 << b)) != 0; }
        inline void setButtonDown(int b) { down |=   1 << b;  }
        inline void setButtonUp(int b)   { down &= ~(1 << b); }
    };
    static Info info;

    static void update();

    static inline void button_callback(GLFWmousebuttonfun f) { glfwSetMouseButtonCallback(Window::window, f); }
    static inline void move_callback(GLFWcursorposfun f)     { glfwSetCursorPosCallback(Window::window, f); }
    static inline void scroll_callback(GLFWscrollfun f) { glfwSetScrollCallback(Window::window, f); }

    static void default_button(GLFWwindow* window, int button, int action, int mods);
    static void default_move(GLFWwindow* window, double x, double y);
    static void default_scroll(GLFWwindow* window, double xoffset, double yoffset);
};

inline std::string getEnvVar(const std::string& var) {
    auto val = std::getenv(var.c_str());
    return std::string(val ? val : "");
}