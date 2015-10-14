#pragma once

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "glm/gtx/transform.hpp"

#include "Entity.h"

const float PI = 3.141592f;
const float CAM_FOV = 2 * PI / 5;

class Camera :
	public Entity
{
public:
	Camera(GLuint shaderProg, GLFWwindow* w);
	~Camera();
	GLFWwindow* window;
	void update(double dt);
	void draw();
	float zoom;
	glm::vec3 getForward();
	glm::vec3 getUp();
	glm::vec3 getRight();
	void turn(float dx,float dy);
	glm::vec3 getLookAt();
	void updateProjection();

private:
	glm::mat4 projection;
	GLuint cameraMatrix;
};

