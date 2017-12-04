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

namespace {
    thread_local Time::time_point frameBeginTime = Time::start;
    thread_local size_t frameCountTime = 0;
    thread_local double deltaTime = 0.;
    thread_local double elapsedTime = 0.;
}

thread_local const Time::time_point& Time::frameBegin = frameBeginTime;
thread_local const size_t& Time::frameCount = frameCountTime;
thread_local const double& Time::delta = deltaTime;
thread_local double Time::nextDeltaOffset = 0;
thread_local const double& Time::elapsed = elapsedTime;

void Time::update() {
    using namespace std::chrono;

    auto currFrameBegin = now();
    deltaTime = get_duration(frameBeginTime, currFrameBegin) + nextDeltaOffset;
    nextDeltaOffset = 0;

    elapsedTime += deltaTime;
    frameBeginTime = currFrameBegin;
    ++frameCountTime;
}

std::mutex UpdateBase::mut;
std::condition_variable UpdateBase::exitCondition;
std::vector<std::thread*> UpdateBase::updateThreads;

bool GLFWmanager::initialized = false;
GLFWwindow* GLFWmanager::hidden_context = nullptr;

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

    hidden_context = glfwCreateWindow(1, 1, "", nullptr, Window::window);
    if (!hidden_context) exit('h');

    // Center the window
    glfwSetWindowPos(Window::window, (vm->width - width) / 2, (vm->height - height) / 2);

    Window::closeCallback(Window::defaultClose);
    Window::resizeCallback(Window::defaultResize);
    Window::focusCallback(Window::defaultFocus);
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
    GLattrarr::setMaxColorAttachments();
    GLstate<GL_VIEWPORT>{}.fetch();
    GLstate<GL_SCISSOR_TEST, GL_SCISSOR_BOX>{}.fetch();
}

namespace {
    safe_queue<std::packaged_task<void()>> mainCommands;

    safe_queue<std::packaged_task<void()>> gfxCommands;

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

std::future<void> Thread::JobGfx::runAsync(const std::function<void()>& func) { 
    // return a completed future if the program is exiting
    if (exiting) {
        return completed_future();
    }
    std::packaged_task<void()> task(func);
    auto future = task.get_future();
    gfxCommands.push(std::move(task)); 
    return future;
}

void Thread::JobGfx::tryExecute() {
    using namespace std::chrono_literals;
    constexpr auto duration = 10ms;

    std::packaged_task<void()> command;
    if (gfxCommands.tryPop(command, duration))
        command();
}

void Thread::JobGfx::flush() {
    exiting = true;
    while (!gfxCommands.empty())
        gfxCommands.pop()();
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
    frameEndCondition.wait(lock);
}

namespace {
    bool windowIsFullScreen;
    bool windowIsInFocus = true;
    Time::time_point windowFocusLostTime;
}

GLFWwindow* Window::window;
const bool& Window::isFullScreen = windowIsFullScreen;
const bool& Window::isInFocus = windowIsInFocus;
const Time::time_point& Window::focusLostTime = windowFocusLostTime;

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
    if (windowIsFullScreen = !windowIsFullScreen)
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

void Window::defaultClose(GLFWwindow* window) {
    static uint32_t close_id = Message::add("window_close");
    Dispatcher::central_trigger.sendBulkEvent<CloseHandler>(close_id);
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

void Window::defaultFocus(GLFWwindow* window, int gainedFocus) {
    windowIsInFocus = gainedFocus == GLFW_TRUE;
    if(!windowIsInFocus) windowFocusLostTime = Time::now();

    static uint32_t focus_id = Message::add("window_focus");
    Dispatcher::central_trigger.sendBulkEvent<FocusHandler>(focus_id, windowIsInFocus, windowFocusLostTime);
}

void Mouse::update() {
    info.prev.x = glm::mix(info.prev.x, info.curr.x, 0.15);
    info.prev.y = glm::mix(info.prev.y, info.curr.y, 0.15);

    for (auto& button : info.buttons) 
        button.downThisFrame = false;

    info.wheel.update((float)Time::delta);
}

void Mouse::defaultButton(GLFWwindow* window, int rawButton, int rawAction, int rawMods) {
    Mouse::Button button{ rawButton };
    bool press = rawAction; // only values are RELEASE and PRESS == false and true
    Keyboard::Key::ModBit mods{ rawMods };

    if (press) {
        info.setButtonDown(button);

        auto& b = info.buttons[rawButton];
        b.lastDown = Time::elapsed;
        b.downThisFrame = true;
    }
    else {
        info.setButtonUp(button);
    }

    static uint32_t button_id = Message::add("mouse_button");
    Dispatcher::central_trigger.sendBulkEvent<ButtonHandler>(button_id, button, press, mods);
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
    info.wheel.vertical = (float)yoffset;
    info.wheel.horizontal = (float) xoffset;

    static uint32_t scroll_id = Message::add("mouse_scroll");
    Dispatcher::central_trigger.sendBulkEvent<ScrollHandler>(scroll_id);
}

std::shared_mutex keyboardMut;

static constexpr size_t getKeyIndex(const int key) { return key - (int)Keyboard::Key::Code::First; }

bool Keyboard::keyPressed(const Key::Code code) { std::shared_lock<std::shared_mutex> lock(keyboardMut); return info.keys[getKeyIndex((int)code)].pressed; }
bool Keyboard::keyDown(const Key::Code code)    { std::shared_lock<std::shared_mutex> lock(keyboardMut); return info.keys[getKeyIndex((int)code)].held; }

bool Keyboard::shiftDown()   { return keyDown(Key::Code::RShift)   || keyDown(Key::Code::LShift);   }
bool Keyboard::controlDown() { return keyDown(Key::Code::LControl) || keyDown(Key::Code::RControl); }
bool Keyboard::altDown()     { return keyDown(Key::Code::LAlt)     || keyDown(Key::Code::RAlt);     }
bool Keyboard::superDown()   { return keyDown(Key::Code::LSuper)   || keyDown(Key::Code::RSuper);   }

void Keyboard::update() {
    std::lock_guard<std::shared_mutex> lock(keyboardMut);
    for (auto& key : info.keys) {
        key.pressed = false;
    }
}

void Keyboard::defaultKey(GLFWwindow* window, int rawKey, int scancode, int rawAction, int rawMods) {
    // works when combined with the scan code, this is difficult to support generally
    Key::Code key{ rawKey };
    if (key == Key::Code::ScanKey) return;

    Key::Action action{ rawAction };
    Key::ModBit mods{ rawMods };

    bool press = action != Key::Action::Release;
    {
        std::lock_guard<std::shared_mutex> lock(keyboardMut);
        auto& keyData = info.keys[getKeyIndex(rawKey)];
        keyData = { keyData.pressed || press, press, Time::elapsed };
    }

    static uint32_t key_id = Message::add("keyboard_key");
    Dispatcher::central_trigger.sendBulkEvent<KeyHandler>(key_id, key, scancode, action, mods);
}
