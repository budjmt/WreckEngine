#include "Camera.h"

Camera* Camera::main = nullptr;

Camera::Camera(GLprogram shaderProg)
{
	if (!main) main = this;
	updateProjection();
	cameraMatrix = shaderProg.getUniform<mat4>("cameraMatrix");
}

void Camera::updateCamMat(GLuniform<mat4>& camLoc) { camLoc.update(projection * view); }

void Camera::update(double dt) {
	view = glm::lookAt(transform.position(), getLookAt(), getUp());
	//update projection
	//updateProjection();
}

void Camera::draw() {
	//does NOTHING because it's a CAMERA
	//or maybe there's debug here
	//who knows
}

void Camera::turn(float dx, float dy) {
	transform.rotate(dy, dx, 0);
}

vec3 Camera::getLookAt(float units) {
	return transform.position() + getForward() * units;
}

void Camera::updateProjection() {
	constexpr auto znear = 0.01f;
	constexpr auto zfar  = 1000.f;
	projection = glm::perspective(CAM_FOV, Window::aspect, znear, zfar);
}

vec3 Camera::getForward() { return transform.forward(); }
vec3 Camera::getUp() {	return transform.up(); }
vec3 Camera::getRight() { return transform.right(); }

void Camera::mayaCam(Camera* camera, double delta) {
	
	auto dt = (float)delta;

	auto mouse = Mouse::info;
	if (mouse.down) {
		// mouse coords are represented in screen coords
		auto dx = (float)(mouse.curr.x - mouse.prev.x) * dt;
		auto dy = (float)(mouse.curr.y - mouse.prev.y) * dt;

		if (mouse.getButtonState(GLFW_MOUSE_BUTTON_LEFT)) {
			auto rot = PI * dt;

			dx = signf(dx) * dx * dx * rot;
			dy = signf(dy) * dy * dy * rot;

			dx = minf(dx, PI * 0.5f);
			dy = minf(dy, PI * 0.5f);

			auto look = camera->getLookAt();
			camera->turn(dx, dy);
			camera->transform.position = look - camera->getForward();
		}
		else if (mouse.getButtonState(GLFW_MOUSE_BUTTON_RIGHT)) {
			camera->transform.position += (dx + dy) * 0.5f * camera->getForward();
		}
		else if (mouse.getButtonState(GLFW_MOUSE_BUTTON_MIDDLE)) {
			camera->transform.position += camera->getRight() * -dx + camera->getUp() * dy;
		}
	}

	constexpr auto u = 5.f;
	if      (Window::getKey(GLFW_KEY_W) == GLFW_PRESS) camera->transform.position += camera->getForward() *  (u * dt);
	else if (Window::getKey(GLFW_KEY_S) == GLFW_PRESS) camera->transform.position += camera->getForward() * -(u * dt);
	if      (Window::getKey(GLFW_KEY_D) == GLFW_PRESS) camera->transform.position += camera->getRight()   * -(u * dt);
	else if (Window::getKey(GLFW_KEY_A) == GLFW_PRESS) camera->transform.position += camera->getRight()   *  (u * dt);

	if      (Window::getKey(GLFW_KEY_UP)    == GLFW_PRESS) camera->transform.position += vec3(0, 1, 0) *  (u * dt);
	else if (Window::getKey(GLFW_KEY_DOWN)  == GLFW_PRESS) camera->transform.position += vec3(0, 1, 0) * -(u * dt);
	if      (Window::getKey(GLFW_KEY_RIGHT) == GLFW_PRESS) camera->transform.position += vec3(1, 0, 0) *  (u * dt);
	else if (Window::getKey(GLFW_KEY_LEFT)  == GLFW_PRESS) camera->transform.position += vec3(1, 0, 0) * -(u * dt);
}