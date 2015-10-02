#include "Camera.h"

Camera::Camera(GLuint shaderProg, GLFWwindow* w)
{
	window = w;

	updateProjection();

	cameraMatrix = glGetUniformLocation(shaderProg, "cameraMatrix");
}

Camera::~Camera()
{
}

void Camera::update(double dt) {
	glm::mat4 view = glm::lookAt(transform.position,getLookAt(),glm::vec3(0,1,0));
	//update projection
	//updateProjection();
	glm::mat4 tmp = projection * view;
	glUniformMatrix4fv(cameraMatrix,1,GL_FALSE,&(tmp[0][0]));
}

void Camera::draw() {
	//does NOTHING because it's a CAMERA
	//or maybe there's debug here
	//who knows
}

void Camera::turn(float dx, float dy) {
	pitch += dy;
	glm::clamp(pitch, - PI / 2, PI / 2);
	yaw += dx;
}

glm::vec3 Camera::getLookAt() {
	return transform.position + getForward();
}

void Camera::updateProjection() {
	int width, height;
	glfwGetWindowSize(window, &width, &height);

	float znear = 0.01f;
	float zfar = 1000.f;
	projection = glm::perspective(CAM_FOV, width * 1.f / height, znear, zfar);
}

glm::vec3 Camera::getForward() {
	glm::mat4 m = glm::rotate(pitch, glm::vec3(1, 0, 0)) * glm::rotate(yaw, glm::vec3(0, 1, 0));
	return (glm::vec3)(m * glm::vec4(0, 0, 1, 1));
}

glm::vec3 Camera::getUp() {
	glm::mat4 m = glm::rotate(pitch, glm::vec3(1, 0, 0)) * glm::rotate(yaw, glm::vec3(0, 1, 0));
	return (glm::vec3)(m * glm::vec4(0, 1, 0, 1));
}

glm::vec3 Camera::getRight() {
	return glm::cross(getForward(), getUp());
}