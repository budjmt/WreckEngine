#pragma once
#define _USE_MATH_DEFINES

#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include "smart_ptr.h"

#include "Entity.h"
#include "DrawDebug.h"

#include <vector>
#include <cmath>

class Game
{
public:
	Game();
	Game(GLprogram prog);
	Entity* operator[](size_t index);
	virtual void update(double dt);
	void draw();
protected:
	GLprogram shader;
	std::vector<shared<Entity>> entities;
	std::vector<shared<Drawable>> shapes;
};

