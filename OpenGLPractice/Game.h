#pragma once
#define _USE_MATH_DEFINES

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "Entity.h"
#include "Shape.h"
#include "DrawDebug.h"

#include <vector>
#include <cmath>

const bool DEBUG = true;

class Game
{
public:
	Game();
	Game(GLuint prog);
	Game(const Game& other);
	~Game(void);
	Entity* operator[](int index);
	virtual void update(double dt);
	void draw();
protected:
	std::vector<Entity*> entities;
	std::vector<Drawable*> shapes;
	DrawDebug* debug;
};

