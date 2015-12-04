#pragma once

#include "GL/glew.h"
#include "glm/glm.hpp"

#include <vector>

#include "Drawable.h"
#include "ShaderHelper.h"

const bool DEBUG = true;

class DrawDebug
{
public:
	static DrawDebug& getInstance();

	//this is the actual draw call
	void draw();

	//these are called externally for drawing stuff
	void drawDebugVector(glm::vec3 start, glm::vec3 end);
private:
	DrawDebug();
	DrawDebug(const DrawDebug&) = delete;
	void operator=(const DrawDebug&) = delete;

	GLuint vecShader;

	GLuint vecVAO, arrowVAO;
	GLuint vecBuffer, arrowBuffer;
	std::vector<glm::vec3> debugVectors;
};

