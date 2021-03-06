#pragma region Local Globals

//#include <vld.h>
#include "GLmanager.h"
#include "smart_ptr.h"

// We do these things not because we hate ourselves, but because C++ hates everyone

unique<GLFWmanager> glfw;
unique<GLEWmanager> glew;

#pragma endregion

#include <iostream>

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"

#include "GLDebug.h"
#include "ShaderHelper.h"
#include "ModelHelper.h"
#include "External.h"
#include "Render.h"
#include "UI.h"

#include "Update.h"
#include "HotSwap.h"

#include "TriPlay.h"
#include "UiTest.h"
#include "TessellatorTest.h"
#include "SimpleGame.h"

void initGraphics();

constexpr int samples = 10;
struct {
    double FPS = 60;
    double runningAvgDelta = 1.0 / FPS;
    bool fpsMode = true;
} fpsInfo;

unique<Game> game;

void init() {
    if (DEBUG)
        initDebug();

    Mouse::defaultMove(Window::window, 0, 0); // this is cheating but it works for initializing the mouse

    initGraphics();

    UI::Initialize();
    Text::init();
    Render::Renderer::init(6);

    // this won't be initialized until after GLFW/GLEW are
    game = make_unique<TriPlay>();
    //game = make_unique<UiTest>();
    //game = make_unique<TessellatorTest>();
    //game = make_unique<SimpleGame>();

    DrawDebug::get().flush();
    Text::flush();
}

void initGraphics() {
    // alpha blending
    GLstate<GL_BLEND, GL_ENABLE_BIT>{ true }.apply();
    GLstate<GL_BLEND, GL_BLEND_FUNC>{ GLstate_comp{ GL_SRC_ALPHA }, GLstate_comp{ GL_ONE_MINUS_SRC_ALPHA } }.apply();

    // texture filtering
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

    // wraps UVs
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));

    // depth buffering
    GLstate<GL_DEPTH_TEST, GL_ENABLE_BIT>{ true }.apply();
    GLstate<GL_DEPTH_TEST, GL_DEPTH_FUNC>{ GL_LEQUAL }.apply();

    // back-face culling
    if (!DEBUG)
        GLstate<GL_CULL_FACE, GL_ENABLE_BIT>{ true }.apply();
}

void physicsUpdate() {
    game->physicsUpdate(Time::delta);
    game->postUpdate();
}

void update() {
    Thread::Main::run(&glfwPollEvents);

    // game update occurs before external updates
    // this enables simpler rules for clearing events per frame
    game->update(Time::delta);
    game->postUpdate();

    std::cout << std::flush; // flush all buffered output at least once per frame

    if (Keyboard::keyPressed(Keyboard::Key::Code::F11))
        Thread::Main::runAsync([] { Window::toggleFullScreen(); });
    if (Keyboard::keyPressed(Keyboard::Key::Code::F10)) {
        static bool vsync = true;
        vsync = !vsync;
        Thread::Render::runNextFrame([=]() { Window::setVsync(vsync); });
    }

    if (Keyboard::altDown()) {
        if (Keyboard::keyPressed(Keyboard::Key::Code::_0))
            fpsInfo.fpsMode = !fpsInfo.fpsMode;
        else if (Keyboard::keyPressed(Keyboard::Key::Code::Equal))
            fpsInfo.FPS += (fpsInfo.FPS < 5) ? 1 : ((fpsInfo.FPS < 20) ? 5 : ((fpsInfo.FPS < 60) ? 10 : 0));
        else if (Keyboard::keyPressed(Keyboard::Key::Code::Minus))
            fpsInfo.FPS -= (fpsInfo.FPS > 20) ? 10 : ((fpsInfo.FPS > 5) ? 5 : ((fpsInfo.FPS > 1) ? 1 : 0));
    }

    Mouse::update();
    Keyboard::update();
}

void updateFPS() {
    fpsInfo.runningAvgDelta += (Time::delta - fpsInfo.runningAvgDelta) / samples;
    auto title = std::to_string(fpsInfo.fpsMode ? 1.0 / fpsInfo.runningAvgDelta : fpsInfo.runningAvgDelta * 1000.0);
    auto decimal = title.find('.');
    if (title.length() - decimal > 3) title = title.erase(decimal + 3);
    title += fpsInfo.fpsMode ? " FpS" : " MSpF";
    auto str = title.c_str();
    Thread::Main::run([str] { glfwSetWindowTitle(Window::window, str); });
}

void draw() {
    updateFPS();

    Thread::Render::executeFrameQueue();

    GLframebuffer::clear();
    game->draw();
    glfwSwapBuffers(Window::window);

    Thread::Render::finishFrame();
}

int main(int argc, char** argv) {
    glfw = make_unique<GLFWmanager>(1280, 720);
    glew = make_unique<GLEWmanager>();

    init();

    // nullify context so it can be moved to the render thread
    glfwMakeContextCurrent(nullptr);

    Update<0>   glJobs(&Thread::JobGfx::tryExecute, [] { glfwMakeContextCurrent(GLFWmanager::hidden_context); });

    Update<0>   regUpdate     (&update);
    Update<120> physicsUpdate (&physicsUpdate);
    Update<0>   render        (&draw, [] { glfwMakeContextCurrent(Window::window); });
    Update<1>   hotSwap       (&HotSwap::main);

    glfwShowWindow(Window::window);
    while (!Window::closing()) {
        Time::update();
        if (!Window::isInFocus) glfwPollEvents(); // the main thread becomes responsible for event polling when the update thread is paused
        Thread::Main::tryExecute();
    }
    Thread::Main::flush();
    Thread::JobGfx::flush();

    UpdateBase::join(); // join all the update threads to ensure destruction
    glfwMakeContextCurrent(Window::window); // ensure that the context is current for global destructors
    return 0;
}
