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
    Camera();

    static Camera* main;

    mat4 getCamMat();
    void update(double dt);
    void draw();

    float znear = 0.01f, zfar = 500.f;

    vec3 forward();
    vec3 up();
    vec3 right();
    void turn(float dx, float dy);
    vec3 getLookAt(float units = 1.f);
    void updateProjection();

    bool sphereInFrustum(const vec3 objPos, const float objRadius) const;

    static void mayaControl(Camera* camera, double dt, const float speed = 5.f);

private:
    ACCS_G (public, mat4, projection);
    ACCS_G (public, mat4, view);
    vec4 frustumPlanes[6];

    void updateFrustum();

    void resizeFunc(Event::Handler::param_t e) {
        updateProjection();
    }
    Event::Handler resizeHandler = Event::make_handler<Window::ResizeHandler>(Event::Handler::wrap_member_func(this, &Camera::resizeFunc));
};