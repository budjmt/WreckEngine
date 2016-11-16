#include "gl_structs.h"
#include "GLmanager.h"
#include "External.h"

#include "DrawDebug.h"

using namespace Event;

// this file is used to define any external stuff needed by all the various utility files

bool GLFWmanager::initialized = false;
bool GLEWmanager::initialized = false;

GLFWmanager::GLFWmanager(const size_t width, const size_t height) {

    if (initialized) return;

    auto val = glfwInit();
    initialized = val != 0;
    if (!initialized) exit(val);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();

    /*const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);*/

    if (DEBUG) glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    Window::window = glfwCreateWindow(width, height, "Wreck Engine", nullptr, nullptr);
    if (!Window::window) exit('w');
    glfwMakeContextCurrent(Window::window);
    Window::default_resize(Window::window, width, height);

    // Center the window
    const GLFWvidmode* vm = glfwGetVideoMode(glfwGetPrimaryMonitor());
    glfwSetWindowPos(Window::window, (vm->width - width) / 2, (vm->height - height) / 2);

    Window::resize_callback(Window::default_resize);
    Mouse::button_callback(Mouse::default_button);
    Mouse::move_callback(Mouse::default_move);
    Mouse::scroll_callback(Mouse::default_scroll);

    Window::cursorMode = GLFW_CURSOR_NORMAL;
    glfwSetInputMode(Window::window, GLFW_CURSOR, Window::cursorMode);

    GLtexture::setMaxTextures();
}

GLFWwindow* Window::window;
int Window::width;
int Window::height;
int Window::frameWidth;
int Window::frameHeight;
vec2 Window::size;
vec2 Window::frameScale;
float Window::aspect;
int Window::cursorMode = GLFW_CURSOR_NORMAL;

Mouse::Info Mouse::info;

void Window::default_resize(GLFWwindow* window, int width, int height) {
    Window::width = width;
    Window::height = height;
    Window::aspect = (float)width / (float)height;
    glfwGetFramebufferSize(window, &Window::frameWidth, &Window::frameHeight);

    Window::size = vec2(width, height);
    Window::frameScale = vec2(width  > 0 ? ((float)frameWidth  / width ) : 0,
                              height > 0 ? ((float)frameHeight / height) : 0);
}

void Mouse::update() {
    info.prev.x = glm::mix(info.prev.x, info.curr.x, 0.15);
    info.prev.y = glm::mix(info.prev.y, info.curr.y, 0.15);
    
    for (int i = 0; i < 3; ++i) 
        info.buttons[i].downThisFrame = false;
    info.wheel = 0.f;
}

void Mouse::default_button(GLFWwindow* window, int button, int action, int mods) {
    if (action == GLFW_PRESS) {
        info.setButtonDown(button);
        
        auto& b = info.buttons[button];
        b.lastDown = glfwGetTime();
        b.downThisFrame = true;
    }
    else if (action == GLFW_RELEASE)
    {
        info.setButtonUp(button);
    }
}

void Mouse::default_move(GLFWwindow* window, double x, double y) {
    // retrieves the mouse coordinates in screen-space, relative to top-left corner
    info.prev = info.curr;
    info.curr.x =   2 * x / Window::width  - 1;
    info.curr.y = -(2 * y / Window::height - 1);
    
    info.currPixel.x = x;
    info.currPixel.y = y;
}

void Mouse::default_scroll(GLFWwindow* window, double xoffset, double yoffset) {
    info.wheel += (float) yoffset;
}