#pragma once

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "glm/gtx/transform.hpp"

#include "Entity.h"

class Camera :
	public Entity
{
public:
	Camera(GLuint shaderProg);
	~Camera();
	GLFWwindow* window;
	void update(double dt);
	void draw();
	float zoom;
	GLfloat yaw, pitch;
	glm::vec3 getForward();
	glm::vec3 getUp();
	glm::vec3 getRight();
	void turn(float dx,float dy);
	glm::vec3 getLookAt();
	void updateProjection();
	glm::mat4 orthographic(float znear, float zfar, int width, int height);
	glm::mat4 perspective(float znear, float zfar, int width, int height);
private:
	glm::mat4 projection;
	GLuint cameraMatrix;
};

