#include "Event.h"

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
    Window::defaultResize(Window::window, width, height);

    // Center the window
    const GLFWvidmode* vm = glfwGetVideoMode(glfwGetPrimaryMonitor());
    glfwSetWindowPos(Window::window, (vm->width - width) / 2, (vm->height - height) / 2);

    Window::resizeCallback(Window::defaultResize);
    Mouse::buttonCallback(Mouse::defaultButton);
    Mouse::moveCallback(Mouse::defaultMove);
    Mouse::scrollCallback(Mouse::defaultScroll);

    Window::cursorMode = GLFW_CURSOR_NORMAL;
    glfwSetInputMode(Window::window, GLFW_CURSOR, Window::cursorMode);

    GLtexture::setMaxTextures();
    GLframebuffer::setMaxColorAttachments();
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

void Window::defaultResize(GLFWwindow* window, int w, int h) {
    width = w;
    height = h;
    aspect = (float) width / height;
    glfwGetFramebufferSize(window, &frameWidth, &frameHeight);

    size = vec2(width, height);
    frameScale = vec2(width  > 0 ? ((float) frameWidth  / width ) : 0,
                      height > 0 ? ((float) frameHeight / height) : 0);

    static uint32_t resize_id = Message::add("window_resize");
    Dispatcher::central_trigger.sendBulkEvent<ResizeHandler>(resize_id);
}

void Mouse::update() {
    info.prev.x = glm::mix(info.prev.x, info.curr.x, 0.15);
    info.prev.y = glm::mix(info.prev.y, info.curr.y, 0.15);
    
    for (int i = 0; i < 3; ++i) 
        info.buttons[i].downThisFrame = false;
    info.wheel = 0.f;
}

void Mouse::defaultButton(GLFWwindow* window, int button, int action, int mods) {
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

    static uint32_t button_id = Message::add("mouse_button");
    Dispatcher::central_trigger.sendBulkEvent<ButtonHandler>(button_id, button, action, mods);
}

void Mouse::defaultMove(GLFWwindow* window, double x, double y) {
    // retrieves the mouse coordinates in screen-space, relative to top-left corner
    info.prev = info.curr;
    info.curr.x =   2 * x / Window::width  - 1;
    info.curr.y = -(2 * y / Window::height - 1);
    
    info.currPixel.x = x;
    info.currPixel.y = y;

    static uint32_t move_id = Message::add("mouse_move");
    Dispatcher::central_trigger.sendBulkEvent<MoveHandler>(move_id);
}

void Mouse::defaultScroll(GLFWwindow* window, double xoffset, double yoffset) {
    info.wheel += (float) yoffset;

    static uint32_t scroll_id = Message::add("mouse_scroll");
    Dispatcher::central_trigger.sendBulkEvent<ScrollHandler>(scroll_id);
}