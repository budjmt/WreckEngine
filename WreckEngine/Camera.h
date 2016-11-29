#pragma once

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "glm/gtx/transform.hpp"

#include "Entity.h"
#include "External.h"

const float CAM_FOV = 2 * PI / 5;

class Camera : public Entity
{
public:
	Camera(GLprogram shaderProg);
	
	static Camera* main;

	mat4 getCamMat();
	void update(double dt);
	void draw();

	float zoom;
	vec3 forward();
	vec3 up();
	vec3 right();
	void turn(float dx,float dy);
	vec3 getLookAt(float units = 1.f);
	void updateProjection();

	static void mayaCam(Camera* camera, double dt);

private:
	mat4 projection, view;
};

