#pragma once

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "glm/gtx/transform.hpp"

#include "Entity.h"
#include "Mouse.h"

const float CAM_FOV = 2 * PI / 5;

class Camera : public Entity
{
public:
	Camera(GLprogram shaderProg, GLFWwindow* w);
	GLFWwindow* window;
	
	void updateCamMat(GLuniform<mat4> camLoc);
	void update(double dt);
	void draw();

	float zoom;
	vec3 getForward();
	vec3 getUp();
	vec3 getRight();
	void turn(float dx,float dy);
	vec3 getLookAt(float units = 1.f);
	void updateProjection();

	static void mayaCam(GLFWwindow* window, Mouse* m, double dt, Camera* camera);

private:
	mat4 projection, view;
	GLuniform<mat4> cameraMatrix;
};

