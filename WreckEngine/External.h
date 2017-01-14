#pragma once

#include "GLFW/glfw3.h"
#include "glm/glm.hpp"

#include "Time.h"
#include "Random.h"

#include <functional>
#include <future>

namespace Thread {
    // main thread functions are typically related to input and other GLFW functionality
    // they can be run synchronously or asynchronously, depending on the requirements, 
    // e.g. input polling must be synchronous while full screening doesn't need to be
    namespace Main {
        std::future<void> runAsync(std::function<void()> func);
        inline void run(std::function<void()> func) { runAsync(func).wait(); }
        void tryExecute(); // intended only to be executed by the main thread
        void flush(); // flushes the remainder of the command queue to free up any pending calls
    };

    // render (aka GL context) thread calls fall into two categories: the actual render thread (the one that is responsible for draw commands) and those from other threads
    // only the render thread has guaranteed execution over a frame in a more or less real-time fashion, other threads use a pre-frame hook for state changes
    namespace Render {
        void runPreFrame(std::function<void()> func); // the commands can originate from any thread
        void executePreFrame(); // executes all pre-frame commands in the buffer
        void finishFrame(); // call this from the render thread to indicate the frame is complete; this allows the loop to cycle
        void waitForFrame(); // call from any thread to wait for the completion of a render frame
    };
}

namespace Window {
    extern GLFWwindow* window;
    extern bool isFullScreen;
    extern int width, height;
    extern int frameWidth, frameHeight;
    extern float aspect;
    extern int cursorMode;

    // These two are purely for convenience with ImGui
    extern vec2 size;
    extern vec2 frameScale;

    inline bool closing() { return glfwWindowShouldClose(Window::window) != 0; }

    void toggleFullScreen(GLFWmonitor* monitor, int x, int y, int width, int height);
    void toggleFullScreen(int width, int height);
    void toggleFullScreen();

    inline void viewport(size_t x, size_t y, size_t width, size_t height) { GL_CHECK(glViewport(x, y, width, height)); }
    inline void viewport(size_t width, size_t height) { viewport(0, 0, width, height); }

    inline void resizeCallback(GLFWwindowsizefun f) { glfwSetWindowSizeCallback(window, f); }

    void defaultResize(GLFWwindow* window, int width, int height);

    class ResizeHandler {}; // inherit or create handler of type to handle window resize events
};

namespace Mouse {
    // this style enables copies to be made, for better concurrency
    struct Info {
        struct button { double lastDown = 0; bool downThisFrame = false; };
        struct cursor { double x = 0, y = 0; };

        uint32_t down = 0; // bit-field
        button buttons[3];
        cursor curr, currPixel, prev;
        const double clickCoolDown = 0.2;
        float wheel = 0.f;

        inline bool getButtonState(int b) { return buttons[b].downThisFrame || (down & (1 << b)) != 0; }
        inline void setButtonDown(int b) { down |=   1 << b;  }
        inline void setButtonUp(int b)   { down &= ~(1 << b); }
    };
    extern Info info;

    void update();

    inline void buttonCallback (GLFWmousebuttonfun f) { glfwSetMouseButtonCallback (Window::window, f); }
    inline void moveCallback   (GLFWcursorposfun f)   { glfwSetCursorPosCallback   (Window::window, f); }
    inline void scrollCallback (GLFWscrollfun f)      { glfwSetScrollCallback      (Window::window, f); }

    void defaultButton(GLFWwindow* window, int button, int action, int mods);
    void defaultMove(GLFWwindow* window, double x, double y);
    void defaultScroll(GLFWwindow* window, double xoffset, double yoffset);

    class ButtonHandler {}; // inherit or create handler of type to handle mouse button events
    class MoveHandler   {}; // inherit or create handler of type to handle mouse move events
    class ScrollHandler {}; // inherit or create handler of type to handle mouse scroll events
};

namespace Keyboard {

    namespace Key {

        struct Info {
            bool pressed;
            bool held;
            double lastPress;
        };

        enum Code : int {
            ScanKey = GLFW_KEY_UNKNOWN,

            Space = GLFW_KEY_SPACE,
            Apostrophe = GLFW_KEY_APOSTROPHE,
            Comma = GLFW_KEY_COMMA,
            Minus = GLFW_KEY_MINUS,
            Period = GLFW_KEY_PERIOD,
            Slash = GLFW_KEY_SLASH,

            _0 = GLFW_KEY_0,
            _1, _2, _3, _4, _5, _6, _7, _8, _9,

            Semicolon = GLFW_KEY_SEMICOLON,
            Equal = GLFW_KEY_EQUAL,

            A = GLFW_KEY_A,
            B, C, D, E, F, G, H, I, J, K, L, M, N,
            O, P, Q, R, S, T, U, V, W, X, Y, Z,

            LBracket = GLFW_KEY_LEFT_BRACKET,
            BSlash = GLFW_KEY_BACKSLASH,
            RBracket = GLFW_KEY_RIGHT_BRACKET,

            GraveAccent = GLFW_KEY_GRAVE_ACCENT,
            World1 = GLFW_KEY_WORLD_1,
            World2 = GLFW_KEY_WORLD_2,

            Escape = GLFW_KEY_ESCAPE,
            Enter = GLFW_KEY_ENTER,
            Tab = GLFW_KEY_TAB,
            Backspace = GLFW_KEY_BACKSPACE,
            Insert = GLFW_KEY_INSERT,
            Delete = GLFW_KEY_DELETE,

            Right = GLFW_KEY_RIGHT,
            Left = GLFW_KEY_LEFT,
            Down = GLFW_KEY_DOWN,
            Up = GLFW_KEY_UP,

            PageUp = GLFW_KEY_PAGE_UP,
            PageDown = GLFW_KEY_PAGE_DOWN,
            Home = GLFW_KEY_HOME,
            End = GLFW_KEY_END,
            CapsLock = GLFW_KEY_CAPS_LOCK,
            ScrollLock = GLFW_KEY_SCROLL_LOCK,
            NumLock = GLFW_KEY_NUM_LOCK,
            PrintScreen = GLFW_KEY_PRINT_SCREEN,
            PauseBreak = GLFW_KEY_PAUSE,

            F1 = GLFW_KEY_F1,
            F2, F3, F4, F5, F6, F7, F8, F9, F10,
            F11, F12, F13, F14, F15, F16, F17,
            F18, F19, F20, F21, F22, F23, F24, F25,

            Keypad0 = GLFW_KEY_KP_0,
            Keypad1, Keypad2, Keypad3, Keypad4,
            Keypad5, Keypad6, Keypad7, Keypad8, Keypad9,
            KeypadDecimal = GLFW_KEY_KP_DECIMAL,
            KeypadDivide = GLFW_KEY_KP_DIVIDE,
            KeypadMultiply = GLFW_KEY_KP_MULTIPLY,
            KeypadSubtract = GLFW_KEY_KP_SUBTRACT,
            KeypadAdd = GLFW_KEY_KP_ADD,
            KeypadEnter = GLFW_KEY_KP_ENTER,
            KeypadEqual = GLFW_KEY_KP_EQUAL,

            LShift = GLFW_KEY_LEFT_SHIFT,
            LControl = GLFW_KEY_LEFT_CONTROL,
            LAlt = GLFW_KEY_LEFT_ALT,
            LSuper = GLFW_KEY_LEFT_SUPER,
            RShift = GLFW_KEY_RIGHT_SHIFT,
            RControl = GLFW_KEY_RIGHT_CONTROL,
            RAlt = GLFW_KEY_RIGHT_ALT,
            RSuper = GLFW_KEY_RIGHT_SUPER,
            Menu = GLFW_KEY_MENU,

            First = Space,
            Last = Menu
        };

        enum ModBit {
            Shift = GLFW_MOD_SHIFT,
            Control = GLFW_MOD_CONTROL,
            Alt = GLFW_MOD_ALT,
            Super = GLFW_MOD_SUPER
        };
    };

    struct Info {
        Key::Info keys[Key::Code::Last - Key::Code::First] = {};
    };
    extern Info info;

    void update();

    // checks if the key was first pressed this frame
    bool keyPressed(const Key::Code code);
    
    bool keyDown(const Key::Code code);
    bool shiftDown();
    bool controlDown();
    bool altDown();
    bool superDown();

    inline void keyCallback (GLFWkeyfun f)  { glfwSetKeyCallback(Window::window, f);  }
    inline void charCallback(GLFWcharfun f) { glfwSetCharCallback(Window::window, f); }

    void defaultKey(GLFWwindow* window, int key, int scancode, int action, int mods);

    class KeyHandler {};

    inline int getKeyRaw(const int keyCode) { return glfwGetKey(Window::window, keyCode); }
};

inline std::string getEnvVar(const std::string& var) {
    auto val = std::getenv(var.c_str());
    return std::string(val ? val : "");
}