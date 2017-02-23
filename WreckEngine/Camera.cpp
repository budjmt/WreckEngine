#include "Camera.h"

Camera* Camera::main = nullptr;

Camera::Camera()
{
	if (!main) main = this;
	updateProjection();
}

mat4 Camera::getCamMat() { return projection * view; }

void Camera::update(double dt) {
	view = glm::lookAt(transform.getComputed()->position(), getLookAt(), up());
	// update projection
	// updateProjection();
}

void Camera::draw() {
	// does NOTHING because it's a CAMERA
	// or maybe there's debug here
	// who knows
}

void Camera::turn(float dx, float dy) {
	transform.rotate(dy, dx, 0);
}

vec3 Camera::getLookAt(float units) {
    auto t = transform.getComputed();
	return t->position() + t->forward() * units;
}

void Camera::updateProjection() {
	constexpr auto znear = 0.01f;
	constexpr auto zfar  = 1000.f;
	projection = glm::perspective(CAM_FOV, Window::aspect, znear, zfar);
}

vec3 Camera::forward() { return transform.forward(); }
vec3 Camera::up() {	return transform.up(); }
vec3 Camera::right() { return transform.right(); }

void Camera::mayaCam(Camera* camera, double delta, const float speed) {
	
	auto dt = (float)delta;

	auto mouse = Mouse::info;
	if (mouse.down) {
		// mouse coords are represented in screen coords
		auto dx = (float)(mouse.curr.x - mouse.prev.x);
		auto dy = (float)(mouse.curr.y - mouse.prev.y);

		if (mouse.getButtonState(GLFW_MOUSE_BUTTON_LEFT)) {
			auto rot = PI;

			dx = signf(dx) * dx * dx * rot;
			dy = signf(dy) * dy * dy * rot;

			dx = minf(dx, PI * 0.5f);
			dy = minf(dy, PI * 0.5f);

			auto look = camera->getLookAt();
			camera->turn(dx, dy);
			camera->transform.position = look - camera->forward();
		}
		else if (mouse.getButtonState(GLFW_MOUSE_BUTTON_RIGHT)) {
			camera->transform.position += (dx + dy) * 0.5f * camera->forward();
		}
		else if (mouse.getButtonState(GLFW_MOUSE_BUTTON_MIDDLE)) {
			camera->transform.position += camera->right() * -dx + camera->up() * dy;
		}
	}

    if      (Keyboard::keyDown(Keyboard::Key::W)) camera->transform.position += camera->forward() * (speed * dt);
	else if (Keyboard::keyDown(Keyboard::Key::S)) camera->transform.position -= camera->forward() * (speed * dt);
	if      (Keyboard::keyDown(Keyboard::Key::D)) camera->transform.position -= camera->right()   * (speed * dt);
	else if (Keyboard::keyDown(Keyboard::Key::A)) camera->transform.position += camera->right()   * (speed * dt);

	if      (Keyboard::keyDown(Keyboard::Key::Up))    camera->transform.position += vec3(0, 1, 0) * (speed * dt);
	else if (Keyboard::keyDown(Keyboard::Key::Down))  camera->transform.position -= vec3(0, 1, 0) * (speed * dt);
	if      (Keyboard::keyDown(Keyboard::Key::Left))  camera->transform.position -= vec3(1, 0, 0) * (speed * dt);
	else if (Keyboard::keyDown(Keyboard::Key::Right)) camera->transform.position += vec3(1, 0, 0) * (speed * dt);
}