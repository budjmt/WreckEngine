#pragma once

#include "smart_ptr.h"
#include "gl_structs.h"

struct GLFWmanager;
struct GLEWmanager;
class Game;

// DON'T declare any globals above here
unique<GLFWmanager> glfw;
unique<GLEWmanager> glew;
double FPS = 60;
double runningAvgDelta = 1.0 / FPS;
int samples = 10;
bool fpsMode = true;
GLprogram shaderProg;
double prevFrame;
unique<Game> game;

void initGraphics();
