#include "Camera.h"

Camera::Camera(GLuint shaderProg)
{
	updateProjection();

	cameraMatrix = glGetUniformLocation(shaderProg, "cameraMatrix");
}

Camera::~Camera()
{
}

void Camera::update(double dt) {
	float cp = cos(pitch), sp = sin(pitch), cy = cos(yaw), sy = sin(yaw);
	glm::vec3 x = { cy, 0, -sy };
	glm::vec3 y = { sy * sp, cp, cy * sp };
	glm::vec3 z = { sy * cp, -sp, cp * cy };
	glm::mat4 view = { glm::vec4(x.x,y.x,z.x,1)
		, glm::vec4(x.y,y.y,z.y,1)
		, glm::vec4(x.z,y.z,z.z,1)
		, glm::vec4(-glm::dot(x,transform.position),-glm::dot(y,transform.position),-glm::dot(z,transform.position),1) };
	//update projection
	glUniformMatrix4fv(cameraMatrix,1,GL_FALSE,&(projection * view)[0][0]);
}

void Camera::draw() {
	//does NOTHING because it's a CAMERA
	//or maybe there's debug here
	//who knows
}

void Camera::turn(float dx, float dy) {
	pitch += dy;
	yaw += dx;
}

glm::vec3 Camera::getLookAt() {
	return transform.position + getForward();
}

void Camera::updateProjection() {
	int width, height;
	glfwGetWindowSize(window, &width, &height);

	float znear = 0.05f;
	float zfar = 5;
	projection = orthographic(znear, zfar, width, height);
}

glm::mat4 Camera::orthographic(float znear, float zfar, int width, int height) {
	return glm::translate(glm::vec3(0, 0, (zfar + znear) * -1 / (zfar - znear))) 
		* glm::scale(glm::vec3(1.f / width, 1.f / height, -2 / (zfar - znear)));
}

glm::mat4 Camera::perspective(float znear, float zfar, int width, int height) {
	float fovx, fovy;
	glm::mat4 p = glm::translate(glm::vec3(0, 0, -2 * znear * zfar / (zfar - znear))) 
		* glm::scale(glm::vec3(glm::atan(fovx / 2), glm::atan(fovy / 2), -2 * znear * zfar / (zfar - znear)));
	p[3][3] = 0;
	p[2][3] = -1;
	return p;
}

glm::vec3 Camera::getForward() {
	glm::mat4 m = glm::translate(transform.position) * glm::rotate(pitch, glm::vec3(1, 0, 0)) * glm::rotate(yaw, glm::vec3(0, 1, 0));
	return (glm::vec3)(m * glm::vec4(0, 0, 1, 1));
}

glm::vec3 Camera::getUp() {
	glm::mat4 m = glm::translate(transform.position) * glm::rotate(pitch, glm::vec3(1, 0, 0)) * glm::rotate(yaw, glm::vec3(0, 1, 0));
	return (glm::vec3)(m * glm::vec4(0, 1, 0, 1));
}

glm::vec3 Camera::getRight() {
	return glm::cross(getForward(), getUp());
}