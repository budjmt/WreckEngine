#pragma once

#include "GL/glew.h"
#include "glm/glm.hpp"

#include <vector>

#include "gl_structs.h"

#include "Drawable.h"
#include "Camera.h"
#include "ShaderHelper.h"
#include "ModelHelper.h"
#include "Mesh.h"

#define DEBUG true

const size_t MAX_VECTORS = 2500;
const size_t MAX_SPHERES = 1000;
const size_t MAX_BOXES = 1000;

struct Sphere { vec4 color; vec3 center; float rad; };

class DrawDebug
{
public:
	static DrawDebug& getInstance();
	void camera(Camera* c);

	//this is the actual draw call
	void draw();

	//these are called externally for drawing stuff
	void drawDebugVector(vec3 start, vec3 end, vec3 color = vec3(0.7f, 1, 0));
	void drawDebugSphere(vec3 pos, float rad, vec3 color = vec3(0.8f, 0.7f, 1.f), float opacity = 0.3f);
	//void drawDebugCube(vec3 pos, float l) { drawDebugCube(pos, l, l, l); };
	//void drawDebugCube(vec3 pos, float w, float h, float d);
private:
	DrawDebug();
	DrawDebug(const DrawDebug&) = delete;
	void operator=(const DrawDebug&) = delete;

	//these are to separate the individual processes
	void drawVectors();
	void drawSpheres();

	Camera* cam = nullptr;
	GLuniform<mat4> vecCamLoc, meshCamLoc;

	shared<Mesh> sphere;
	size_t numSphereVerts;

	GLprogram vecShader, meshShader;

	GLVAO vecVAO, arrowVAO, sphereVAO;
	GLbuffer vecBuffer, arrowBuffer;
	GLbuffer sphereBuffer, sphereInstBuffer, sphereElBuffer;
	
	struct m_MeshData { vec4 color; mat4 transform; };

	std::vector<vec3> debugVectors, debugBoxes, arrows;
	std::vector<Sphere> debugSpheres;
	std::vector<m_MeshData> sphereInsts;
};