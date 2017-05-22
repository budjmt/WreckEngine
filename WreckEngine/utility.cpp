#include "Event.h"

#include <shared_mutex>

#include "gl_structs.h"
#include "GLmanager.h"
#include "External.h"

#include "DrawDebug.h"

#include "Update.h"
#include "safe_queue.h"

using namespace Event;

// this file is used to define any external stuff needed by all the various utility files

const std::chrono::high_resolution_clock::time_point Time::start = Time::now();

std::chrono::high_resolution_clock::time_point Time::now() {
    return std::chrono::high_resolution_clock::now();
}

std::chrono::system_clock::time_point Time::system_now() {
    return std::chrono::system_clock::now();
}

long long Time::ctime() {
    using namespace std::chrono;
    return duration_cast<seconds>(Time::now().time_since_epoch()).count();
}

double Time::elapsed() {
    using namespace std::chrono;
    return duration<double>(now() - start).count();
}

thread_local double deltaTime = 0.;
thread_local const double& Time::delta = deltaTime;

void Time::updateDelta() {
    using namespace std::chrono;
    thread_local auto prevFrame = Time::start;
    auto currFrame = now();
    deltaTime = duration<double>(currFrame - prevFrame).count();
    prevFrame = currFrame;
}

std::mutex UpdateBase::mut;
std::condition_variable UpdateBase::exitCondition;
std::vector<std::thread*> UpdateBase::updateThreads;

bool GLFWmanager::initialized = false;
bool GLEWmanager::initialized = false;

GLFWmanager::GLFWmanager(const size_t width, const size_t height) {

    if (initialized) return;

    auto val = glfwInit();
    initialized = val != 0;
    if (!initialized) exit(val);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    auto monitor = getMonitor();
    const auto vm = glfwGetVideoMode(monitor);

    /*
    glfwWindowHint(GLFW_RED_BITS, vm->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, vm->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, vm->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, vm->refreshRate);
    */

    if (DEBUG) glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    Window::window = glfwCreateWindow(width, height, "Wreck Engine", nullptr, nullptr);
    if (!Window::window) exit('w');
    glfwMakeContextCurrent(Window::window);
    Window::defaultResize(Window::window, width, height);

    // Center the window
    glfwSetWindowPos(Window::window, (vm->width - width) / 2, (vm->height - height) / 2);

    Window::resizeCallback(Window::defaultResize);
    Mouse::buttonCallback(Mouse::defaultButton);
    Mouse::moveCallback(Mouse::defaultMove);
    Mouse::scrollCallback(Mouse::defaultScroll);
    Keyboard::keyCallback(Keyboard::defaultKey);

    Window::cursorMode = GLFW_CURSOR_NORMAL;
    glfwSetInputMode(Window::window, GLFW_CURSOR, Window::cursorMode);
}

void GLEWmanager::initGLValues() {
    GLtexture::setMaxTextures();
    GLprogram::setMaxWorkGroups();
    GLframebuffer::setMaxColorAttachments();
}

namespace {
    safe_queue<std::packaged_task<void()>> mainCommands;
    
    safe_queue<std::function<void()>> preRenderCommands;
    std::mutex frameMutex;
    std::condition_variable frameEndCondition;
    
    bool exiting = false;
};

std::future<void> completed_future() {
    std::promise<void> temp;
    temp.set_value();
    return temp.get_future();
}

std::future<void> Thread::Main::runAsync(const std::function<void()>& func) { 
    // return a completed future if the program is exiting
    if (exiting) {
        return completed_future();
    }
    std::packaged_task<void()> task(func);
    auto future = task.get_future();
    mainCommands.push(std::move(task));
    return future; 
}

void Thread::Main::tryExecute() {
    using namespace std::chrono_literals;
    constexpr auto duration = 10ms;

    std::packaged_task<void()> command;
    if (mainCommands.tryPop(command, duration))
        command();
}

void Thread::Main::flush() {
    exiting = true;
    while (!mainCommands.empty())
        mainCommands.pop()();
}

void Thread::Render::runNextFrame(std::function<void()> func) { preRenderCommands.push(func); }

void Thread::Render::executeFrameQueue() {
    while (!preRenderCommands.empty()) {
        preRenderCommands.pop()();
    }
}

void Thread::Render::finishFrame() { frameEndCondition.notify_all(); }

void Thread::Render::waitForFrame() {
    std::unique_lock<std::mutex> lock(frameMutex);
    frameEndCondition.wait(lock, [] { return true; });
}

GLFWwindow* Window::window;
bool Window::isFullScreen;
int Window::width;
int Window::height;
int Window::frameWidth;
int Window::frameHeight;
vec2 Window::size;
vec2 Window::frameScale;
float Window::aspect;
int Window::cursorMode = GLFW_CURSOR_NORMAL;

Mouse::Info Mouse::info;
Keyboard::Info Keyboard::info;

void Window::toggleFullScreen(GLFWmonitor* monitor, int x, int y, int width, int height) {
    if (isFullScreen = !isFullScreen)
        glfwSetWindowMonitor(window, monitor, 0, 0, width, height, GLFW_DONT_CARE);
    else
        glfwSetWindowMonitor(window, nullptr, x, y, width, height, GLFW_DONT_CARE);
}
void Window::toggleFullScreen(int width, int height) {
    auto monitor = GLFWmanager::getMonitor();
    const auto vidMode = glfwGetVideoMode(monitor);
    if(isFullScreen)
        toggleFullScreen(monitor, (vidMode->width - width) / 2, (vidMode->height - height) / 2, width, height);
    else
        toggleFullScreen(monitor, 0, 0, vidMode->width, vidMode->height);
}
void Window::toggleFullScreen() {
    static int oldWidth = width, oldHeight = height;
    if (!isFullScreen) {
        oldWidth = width;
        oldHeight = height;
    }
    toggleFullScreen(oldWidth, oldHeight);
}

void Window::defaultResize(GLFWwindow* window, int w, int h) {
    width = w;
    height = h;
    glfwGetFramebufferSize(window, &frameWidth, &frameHeight);
    aspect = (float) frameWidth / frameHeight;

    size = vec2(width, height);
    frameScale = vec2(width  > 0 ? ((float) frameWidth  / width ) : 0,
                      height > 0 ? ((float) frameHeight / height) : 0);

    Thread::Render::runNextFrame([] { viewport(frameWidth, frameHeight); });

    static uint32_t resize_id = Message::add("window_resize");
    Dispatcher::central_trigger.sendBulkEvent<ResizeHandler>(resize_id);
}

void Mouse::update() {
    info.prev.x = glm::mix(info.prev.x, info.curr.x, 0.15);
    info.prev.y = glm::mix(info.prev.y, info.curr.y, 0.15);
    
    for (int i = 0; i < 3; ++i) 
        info.buttons[i].downThisFrame = false;

    info.wheel.frame = 0.f;
    info.wheel.accum = glm::mix(0.f, info.wheel.accum, maxf(1.f - (float) Time::delta * 100, 0.f));
}

void Mouse::defaultButton(GLFWwindow* window, int button, int action, int mods) {
    if (action == GLFW_PRESS) {
        info.setButtonDown(button);
        
        auto& b = info.buttons[button];
        b.lastDown = Time::elapsed();
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
    info.wheel.frame = info.wheel.accum = (float) yoffset;

    static uint32_t scroll_id = Message::add("mouse_scroll");
    Dispatcher::central_trigger.sendBulkEvent<ScrollHandler>(scroll_id);
}

std::shared_mutex keyboardMut;

constexpr size_t getKeyIndex(const int key) { return key - Keyboard::Key::Code::First; }

bool Keyboard::keyPressed(const Key::Code code) { std::shared_lock<std::shared_mutex> lock(keyboardMut); return info.keys[getKeyIndex(code)].pressed; }
bool Keyboard::keyDown(const Key::Code code)    { std::shared_lock<std::shared_mutex> lock(keyboardMut); return info.keys[getKeyIndex(code)].held; }

bool Keyboard::shiftDown()   { return keyDown(Key::Code::RShift)   || keyDown(Key::Code::LShift);   }
bool Keyboard::controlDown() { return keyDown(Key::Code::LControl) || keyDown(Key::Code::RControl); }
bool Keyboard::altDown()     { return keyDown(Key::Code::LAlt)     || keyDown(Key::Code::RAlt);     }
bool Keyboard::superDown()   { return keyDown(Key::Code::LSuper)   || keyDown(Key::Code::RSuper);   }

void Keyboard::update() {
    std::lock_guard<std::shared_mutex> lock(keyboardMut);
    constexpr size_t k = getKeyIndex(Key::Code::Last);
    for (size_t i = 0; i < k; ++i) {
        info.keys[i].pressed = false;
    }
}

void Keyboard::defaultKey(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // works when combined with the scan code, this is difficult to support generally
    if (key == Key::Code::ScanKey) return;

    bool press = action != GLFW_RELEASE;
    {
        std::lock_guard<std::shared_mutex> lock(keyboardMut);
        auto& keyData = info.keys[getKeyIndex(key)];
        keyData = { keyData.pressed || press, press, Time::elapsed() };
    }

    static uint32_t key_id = Message::add("keyboard_key");
    Dispatcher::central_trigger.sendBulkEvent<KeyHandler>(key_id, key, scancode, action, mods);
}

