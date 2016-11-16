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
#include "TriPlay.h"
#include "UiTest.h"
#include "UI.h"

void initGraphics();

double FPS = 60;
double runningAvgDelta = 1.0 / FPS;
int samples = 10;
bool fpsMode = true;
GLprogram shaderProg;
double prevFrame;
unique<Game> game;

using namespace std;

void init() {
    shaderProg = loadProgram("Shaders/matvertexShader.glsl","Shaders/matfragmentShader.glsl");
    if(shaderProg()) {
        shaderProg.use();
        shaderProg.getUniform<vec4>("tint").update(vec4(1));
    }

    prevFrame = glfwGetTime();
    Mouse::default_move(Window::window, 0, 0);//this is cheating but it works for initializing the mouse

    initGraphics();

    UI::Initialize();
    Text::init();

    // this won't be initialized until after GLFW/GLEW are
    //game = make_unique<TriPlay>(shaderProg);
    game = make_unique<UiTest>();
}

void initGraphics() {
    GL_CHECK(glViewport(0, 0, Window::width, Window::height));

    // alpha blending
    GL_CHECK(glEnable(GL_BLEND));
    GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
        
    // texture filtering
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

    //wraps UVs
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));

    // depth buffering
    GL_CHECK(glEnable(GL_DEPTH_TEST));
    GL_CHECK(glDepthFunc(GL_LESS));

    // back-face culling
    if (!DEBUG) { GL_CHECK(glEnable(GL_CULL_FACE)); }
    GL_CHECK(glCullFace(GL_BACK));
    GL_CHECK(glFrontFace(GL_CCW));

    // render as wire-frame
    //GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
}

void update() {
    auto currFrame = glfwGetTime();
    auto dt = currFrame - prevFrame;
    prevFrame = currFrame;
    //need to separate drawing and update pipelines
    //double spf = 1.0 / FPS;
    //while (dt < spf) {
    //  currFrame = glfwGetTime();
    //  dt += currFrame - prevFrame;
    //  prevFrame = currFrame;
    //}

    runningAvgDelta -= runningAvgDelta / samples;
    runningAvgDelta += dt / samples;
    auto title = std::to_string(fpsMode ? 1.0 / runningAvgDelta : runningAvgDelta * 1000.0);
    auto decimal = title.find('.');
    if (title.length() - decimal > 3) title = title.erase(decimal + 3);
    title += fpsMode ? " FpS" : " MSpF";
    glfwSetWindowTitle(Window::window, title.c_str());

    Mouse::update();

    bool alt = Window::getKey(GLFW_KEY_RIGHT_ALT) == GLFW_PRESS || Window::getKey(GLFW_KEY_LEFT_ALT) == GLFW_PRESS;
    if (alt) {
        if (Window::getKey(GLFW_KEY_0) == GLFW_PRESS)
            fpsMode = !fpsMode;
        if (Window::getKey(GLFW_KEY_EQUAL) == GLFW_PRESS)
            FPS += (FPS < 5) ? 1 : ((FPS < 20) ? 5 : ((FPS < 60) ? 10 : 0));
        if (Window::getKey(GLFW_KEY_MINUS) == GLFW_PRESS)
            FPS -= (FPS > 20) ? 10 : ((FPS > 5) ? 5 : ((FPS > 1) ? 1 : 0));
    }

    game->update(dt);
}

void draw() {
        GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
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
