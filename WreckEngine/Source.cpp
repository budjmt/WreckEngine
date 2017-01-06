#pragma region Local Globals

#include "GLmanager.h"
#include "smart_ptr.h"

// We do these things not because we hate ourselves, but because C++ hates everyone

unique<GLFWmanager> glfw;
unique<GLEWmanager> glew;

#pragma endregion

#include <vld.h>
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
#include "TriPlay.h"
#include "UiTest.h"
#include "UI.h"

void initGraphics();

double FPS = 60;
double runningAvgDelta = 1.0 / FPS;
int samples = 10;
bool fpsMode = true;
GLprogram shaderProg;
unique<Game> game;

using namespace std;

void init() {
    shaderProg = loadProgram("Shaders/matvertexShader.glsl","Shaders/matfragmentShader.glsl");
    if(shaderProg.valid()) {
        shaderProg.use();
        shaderProg.setOnce<vec4>("tint", vec4(1));
    }

    Mouse::defaultMove(Window::window, 0, 0);//this is cheating but it works for initializing the mouse

    initGraphics();

    UI::Initialize();
    Text::init();
    Render::Renderer::init(6);

    // this won't be initialized until after GLFW/GLEW are
    game = make_unique<TriPlay>(shaderProg);
    //game = make_unique<UiTest>();
}

void initGraphics() {
    // alpha blending
    GL_CHECK(glEnable(GL_BLEND));
    GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
        
    // texture filtering
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

    // wraps UVs
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));

    // depth buffering
    GL_CHECK(glEnable(GL_DEPTH_TEST));
    GL_CHECK(glDepthFunc(GL_LEQUAL));

    // back-face culling
    if (!DEBUG)
        GL_CHECK(glEnable(GL_CULL_FACE));
    GL_CHECK(glCullFace(GL_BACK));
    GL_CHECK(glFrontFace(GL_CCW));
}

void update() {
    Time::updateDelta();
    //need to separate drawing and update pipelines
    //double spf = 1.0 / FPS;
    //while (dt < spf) {
    //  currFrame = Time::elapsed();
    //  dt += currFrame - prevFrame;
    //  prevFrame = currFrame;
    //}

    // game update occurs before anything else
    // this enables simpler rules for clearing events per frame
    game->update(Time::delta);

    runningAvgDelta -= runningAvgDelta / samples;
    runningAvgDelta += Time::delta / samples;
    auto title = std::to_string(fpsMode ? 1.0 / runningAvgDelta : runningAvgDelta * 1000.0);
    auto decimal = title.find('.');
    if (title.length() - decimal > 3) title = title.erase(decimal + 3);
    title += fpsMode ? " FpS" : " MSpF";
    glfwSetWindowTitle(Window::window, title.c_str());

    if (Keyboard::keyPressed(Keyboard::Key::F11))
        Window::toggleFullScreen();

    if (Keyboard::altDown()) {
        if (Keyboard::keyPressed(Keyboard::Key::_0))
            fpsMode = !fpsMode;
        else if (Keyboard::keyPressed(Keyboard::Key::Equal))
            FPS += (FPS < 5) ? 1 : ((FPS < 20) ? 5 : ((FPS < 60) ? 10 : 0));
        else if (Keyboard::keyPressed(Keyboard::Key::Minus))
            FPS -= (FPS > 20) ? 10 : ((FPS > 5) ? 5 : ((FPS > 1) ? 1 : 0));
    }

    Mouse::update();
    Keyboard::update();
}

void draw() {
    GLframebuffer::clear();
    game->draw();
}

int main(int argc, char** argv) {
    glfw = make_unique<GLFWmanager>(800, 600);
    glew = make_unique<GLEWmanager>();

    if (DEBUG)
        initDebug();
    init();
    glfwShowWindow(Window::window);
    while (!glfwWindowShouldClose(Window::window)) {
        glfwPollEvents();
        update();
        draw();
        glfwSwapBuffers(Window::window);
    }

    return 0;
}
