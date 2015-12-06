#pragma once

#include "GL/glew.h"
#include "glm/glm.hpp"

#include <vector>

#include "Drawable.h"
#include "Camera.h"
#include "ShaderHelper.h"
#include "ModelHelper.h"
#include "Mesh.h"

const bool DEBUG = true;

struct Sphere {
	glm::vec3 center;
	float rad;
};

class DrawDebug
{
public:
	static DrawDebug& getInstance();
	void camera(Camera* c);

	//this is the actual draw call
	void draw();

	//these are called externally for drawing stuff
	void drawDebugVector(glm::vec3 start, glm::vec3 end, glm::vec3 color = glm::vec3(0.7f, 1, 0));
	void drawDebugSphere(glm::vec3 pos, float rad);
private:
	DrawDebug();
	~DrawDebug();
	DrawDebug(const DrawDebug&) = delete;
	void operator=(const DrawDebug&) = delete;

	//these are to separate the individual processes
	void drawVectors();
	void drawSpheres();

	Camera* cam = nullptr;
	GLuint vecCamLoc, meshCamLoc;

	Mesh* sphere;
	int sphereVerts;
	GLuint worldLoc;

	GLuint vecShader, meshShader;

	GLuint vecVAO, arrowVAO, meshVAO;
	GLuint vecBuffer, arrowBuffer;
	GLuint sphereBuffer, sphereElBuffer;
	
	std::vector<glm::vec3> debugVectors;
	std::vector<Sphere> debugSpheres;
};