#pragma once

#include "GL/glew.h"
#include "glm/glm.hpp"

#include <vector>

class DrawDebug
{
public:
	DrawDebug();
	~DrawDebug();

	static DrawDebug* getInstance();

	//this is the actual draw call
	void draw();

	//these are called externally for drawing stuff
	static void drawDebugVector(glm::vec3 v);
private:
	std::vector<glm::vec3> debugVectors;
};

