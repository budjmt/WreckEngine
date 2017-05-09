#include "Camera.h"

Camera* Camera::main = nullptr;

Camera::Camera()
{
    if (!main) main = this;
    updateProjection();
}

mat4 Camera::getCamMat() { return _projection * _view; }

void Camera::update(double dt) {
    auto t = transform.getComputed();
    _view = glm::lookAt(t->position(), getLookAt(), t->up());
    updateFrustum();
}

//#include "DrawDebug.h"
void Camera::updateFrustum() {
    const auto vp = getCamMat();
    const auto r3 = vec4(vp[0][3], vp[1][3], vp[2][3], vp[3][3]);

    // 0 is left/right, 1 is bottom/top
    for (int i = 0; i < 2; ++i) {
        const auto rCurr = vec4(vp[0][i], vp[1][i], vp[2][i], vp[3][i]);
        const auto index = i * 2 + 1;

        const auto p1 = r3 + rCurr;
        frustumPlanes[index] = p1 / glm::length(vec3(p1));

        const auto p2 = r3 - rCurr;
        frustumPlanes[index + 1] = p2 / glm::length(vec3(p2));
    }

    // 2 is near / far; their positions in the array are optimized for how useful their check is
    const auto r2 = vec4(vp[0][2], vp[1][2], vp[2][2], vp[2][3]);
    
    const auto near = r3 + r2;
    frustumPlanes[0] = near / glm::length(near);

    const auto far = r3 - r2;
    frustumPlanes[5] = far / glm::length(far);
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
    _projection = glm::perspective(CAM_FOV, Window::aspect, znear, zfar);
}

vec3 Camera::forward() { return transform.forward(); }
vec3 Camera::up()      { return transform.up(); }
vec3 Camera::right()   { return transform.right(); }

bool Camera::sphereInFrustum(const vec3 center, const float radius) const {
    const auto center4 = vec4(center, 1);

    for (const auto& plane : frustumPlanes) {
        if (glm::dot(plane, center4) < -radius)
            return false;
    }

    return true;
}

void Camera::mayaControl(Camera* camera, double delta, const float speed) {

    auto dt = (float)delta;

    auto mouse = Mouse::info;
    if (mouse.down) {
        // mouse coords are represented in screen coords
        auto dx = (float)(mouse.curr.x - mouse.prev.x);
        auto dy = (float)(mouse.curr.y - mouse.prev.y);

        if (mouse.getButtonState(GLFW_MOUSE_BUTTON_LEFT)) {

            dx *= 10.f * dt;
            dy *= 10.f * dt;

            dx = 2 * asin(dx * 0.5f);
            dy = 2 * asin(dy * 0.5f);

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

    if      (Keyboard::keyDown(Keyboard::Key::W))     camera->transform.position += camera->forward() * (speed * dt);
    else if (Keyboard::keyDown(Keyboard::Key::S))     camera->transform.position -= camera->forward() * (speed * dt);
    if      (Keyboard::keyDown(Keyboard::Key::D))     camera->transform.position -= camera->right()   * (speed * dt);
    else if (Keyboard::keyDown(Keyboard::Key::A))     camera->transform.position += camera->right()   * (speed * dt);

    if      (Keyboard::keyDown(Keyboard::Key::Up))    camera->transform.position += vec3(0, 1, 0) * (speed * dt);
    else if (Keyboard::keyDown(Keyboard::Key::Down))  camera->transform.position -= vec3(0, 1, 0) * (speed * dt);
    if      (Keyboard::keyDown(Keyboard::Key::Left))  camera->transform.position -= vec3(1, 0, 0) * (speed * dt);
    else if (Keyboard::keyDown(Keyboard::Key::Right)) camera->transform.position += vec3(1, 0, 0) * (speed * dt);
}