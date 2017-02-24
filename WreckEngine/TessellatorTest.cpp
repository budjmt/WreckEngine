#include "TessellatorTest.h"

#include "ColliderEntity.h"
#include "HotSwap.h"
#include "TextEntity.h"

struct RenderData {
    GLprogram prog;
    GLuniform<mat4> mat;
    GLuniform<vec3> pos;
};
static RenderData planetData, planeData, controlData;

static shared<TextEntity> controlText;
static shared<Entity> me, planet, plane;

TessellatorTest::TessellatorTest() : Game(6) {
    auto mainState = make_shared<State>("main");
    addState(mainState);

    controlText = make_shared<TextEntity>("", "arial.ttf", Text::Justify::MIDDLE, Text::Justify::START, 48);
    controlText->transform.position = vec3(25, 50, 0);
    controlText->transform.scale = vec3(0.5f, 1, 1);
    mainState->addEntity(controlText);

    auto tessProg = HotSwap::Shader::create();
    using ShaderRes = decltype(tessProg->vertex);

    tessProg->vertex      = ShaderRes("Shaders/planet_v.glsl",  GL_VERTEX_SHADER);
    tessProg->tessControl = ShaderRes("Shaders/planet_tc.glsl", GL_TESS_CONTROL_SHADER);
    tessProg->tessEval    = ShaderRes("Shaders/planet_te.glsl", GL_TESS_EVALUATION_SHADER);
    tessProg->fragment    = ShaderRes("Shaders/planet_f.glsl",  GL_FRAGMENT_SHADER);
    tessProg->setupProgram();
    planetData.prog = tessProg->getProgram();
    planetData.mat = planetData.prog.getUniform<mat4>("cameraMatrix");
    planetData.pos = planetData.prog.getUniform<vec3>("camPos");

    auto cube = loadOBJ("Assets/cube.obj");

    auto dm = make_shared<DrawMesh>(&renderer.forward.objects, cube, "Assets/texture.png", planetData.prog);
    dm->tesselPrim = GL_PATCHES;
    dm->renderGroup = renderer.forward.objects.addGroup([] {
        GL_CHECK(glEnable(GL_CULL_FACE));
        GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
    }, [] {
        GL_CHECK(glDisable(GL_CULL_FACE));
        GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
    });

    planet = make_shared<Entity>(dm);
    planet->id = (void*)0xabc;
    mainState->addEntity(planet);

    planeData.prog = loadProgram("Shaders/normalize_v.glsl", "Shaders/planet_f.glsl");
    planeData.mat = planeData.prog.getUniform<mat4>("cameraMatrix");

    //genPlane("Assets/plane.obj", 5);
    dm = make_shared<DrawMesh>(&renderer.forward.objects, loadOBJ("Assets/plane.obj"), "Assets/texture.png", planeData.prog);
    dm->renderGroup = 1;

    plane = make_shared<Entity>(dm);
    plane->active = false;
    mainState->addEntity(plane);

    auto camera = make_shared<Camera>();
    camera->id = (void*)0xcab;
    camera->transform.position = vec3(0, 0, 5);
    camera->transform.rotate(0, PI, 0);
    mainState->addEntity(camera);

    controlData.prog = loadProgram("Shaders/matvertexShader.glsl", "Shaders/dumb_f.glsl");
    controlData.mat = controlData.prog.getUniform<mat4>("cameraMatrix");

    auto cone = loadOBJ("Assets/cone.obj");
    cone->rotate(quat::rotation(PI / 2, vec3(1, 0, 0)));
    me = make_shared<Entity>(make_shared<DrawMesh>(&renderer.forward.objects, cone, "Assets/texture.png", controlData.prog));
    me->transform.position = vec3(-3, 0, 0);
    me->color = vec4(1,0,0,1);
    mainState->addEntity(me);

    renderer.lightingOn = false;
    
    if (DEBUG) DrawDebug::getInstance().camera(camera.get());
}

vec3 getClosestSphereDir(vec3, float, vec3, vec3);
void moveCamera(Entity* camera, float radius);
void adjustCamera(Entity* camera, vec3 positionDir, float dist);

void TessellatorTest::update(double delta) {
    const auto dt = (float)delta;

    Game::update(delta);

    //quit the game
    //if (Keyboard::keyDown(Keyboard::Key::Code::Q)) Window::close(); // pretty broken in this game

    DrawDebug::getInstance().drawDebugVector(vec3(), vec3(1, 0, 0), vec3(1, 0, 0));
    DrawDebug::getInstance().drawDebugVector(vec3(), vec3(0, 1, 0), vec3(0, 1, 0));
    DrawDebug::getInstance().drawDebugVector(vec3(), vec3(0, 0, 1), vec3(0, 0, 1));

    const float radius = 2;

    auto cam = me.get();

    moveCamera(cam, radius);
    
    auto pos = cam->transform.position();
    auto dist = glm::length(pos) - radius;
    adjustCamera(cam, pos / (dist + radius), dist);

    pos = Camera::main->transform.position();
    dist = glm::length(pos) - radius;
    if (dist < 2) {
        plane->active = true;
        planet->active = false;

        auto n = getClosestSphereDir(vec3(), radius, pos, Camera::main->transform.getComputed()->forward());
        auto closestToForward = radius * n;
        plane->transform.position = closestToForward;
        
        plane->transform.rotation = quat(rotateBetween(vec3(0, 0, 1), n));
        
        auto forward = plane->transform.forward();
        auto correctUp = vec3(-forward.x * forward.y, -forward.x * forward.x + forward.z * forward.z, -forward.z * forward.y);
        plane->transform.rotation *= quat(rotateBetween(plane->transform.up(), correctUp));

        float scaleFactor;
        if      (dist < 0.25f) scaleFactor = 1;
        else if (dist < 0.75f) scaleFactor = 2;
        else                   scaleFactor = 5;

        plane->transform.scale = vec3(scaleFactor);
    }
    else {
        planet->active = true;
        plane->active = false;
    }

    Camera::mayaCam(Camera::main, dt);

    pos = cam->transform.getComputed()->position();
    controlText->setMessage(to_string(pos, 3) + "\n" + std::to_string(glm::length(pos)) + "\n" + to_string(cam->transform.forward()));
}

void TessellatorTest::postUpdate() {
    Game::postUpdate();
    Text::postUpdate();
}

void TessellatorTest::draw() {
    auto mat = Camera::main->getCamMat();
    planetData.prog.use();
    planetData.mat.update(mat);
    planetData.pos.update(Camera::main->transform.getComputed()->position);
    //planetData.pos.update(me->transform.getComputed()->position);
    planeData.prog.use();
    planeData.mat.update(mat);

    controlData.prog.use();
    controlData.mat.update(mat);

    Game::draw();
    Text::render(&renderer.forward.objects);
}

vec3 getTangent(vec3 normal, vec3 forward) {
    float parallel = glm::dot(forward, normal);
    auto val = forward - parallel * normal;
    auto len = glm::length(val);
    return len ? val / len : glm::cross(normal, vec3(1,0,0));
}

vec3 getClosestSphereDir(vec3 c, float r, vec3 p0, vec3 n) {
    auto u = c - p0;
    auto t = glm::dot(n, u); // normally divided by dot(n, n) == |n|^2, but n is a unit vector so it's unnecessary
    auto s = t * t - glm::dot(u, u) + r * r;
    // if the line intersects the sphere, the closest point to the origin isn't the closest pt to the surface
    if (s >= 0) {
        s = sqrt(s);
        t = t - glm::sign(t) * s; // not 100% predictable when p0 is inside the sphere, but that's fine
    }
    auto closestLinePt = p0 + n * t;
    return glm::normalize(closestLinePt - c); // to get closest pt on sphere, multiply by radius and add to center
}

void moveCamera(Entity* camera, float radius) {
    auto dt = (float)Time::delta;

    //auto mouse = Mouse::info;
    //if (mouse.down) {
    //    // mouse coords are represented in screen coords
    //    auto dx = (float)(mouse.curr.x - mouse.prev.x);
    //    auto dy = (float)(mouse.curr.y - mouse.prev.y);
    //
    //    if (mouse.getButtonState(GLFW_MOUSE_BUTTON_LEFT)) {
    //        auto rot = PI;
    //
    //        dx = signf(dx) * dx * dx * rot;
    //        dy = signf(dy) * dy * dy * rot;
    //
    //        dx = minf(dx, PI * 0.5f);
    //        dy = minf(dy, PI * 0.5f);
    //
    //        auto look = camera->getLookAt();
    //        camera->turn(dx, dy);
    //        camera->transform.position = look - camera->forward();
    //    }
    //    else if (mouse.getButtonState(GLFW_MOUSE_BUTTON_RIGHT)) {
    //        camera->transform.position += (dx + dy) * 0.5f * camera->forward();
    //    }
    //    else if (mouse.getButtonState(GLFW_MOUSE_BUTTON_MIDDLE)) {
    //        camera->transform.position += camera->right() * -dx + camera->up() * dy;
    //    }
    //}

    auto pos = camera->transform.position();
    auto centerDist = glm::length(pos);

    const auto towardSpeed = centerDist - radius;
    const auto lateralSpeed = 5.f;

    const bool shift = Keyboard::shiftDown();
    
    if (shift) {
        // move away from/towards the surface
        if      (Keyboard::keyDown(Keyboard::Key::I))   camera->transform.position += pos * (towardSpeed * dt / centerDist);
        else if (Keyboard::keyDown(Keyboard::Key::K)) camera->transform.position -= pos * (towardSpeed * dt / centerDist);
    }
    else {
        // move relative to the surface
        auto normal  = pos / centerDist; 
        auto forward = camera->transform.forward();
        auto tangent = getTangent(normal, forward);

        auto point = normal * radius;
        DrawDebug::getInstance().drawDebugVector(point, point + tangent);
        
        if      (Keyboard::keyDown(Keyboard::Key::I))     camera->transform.position += tangent * (lateralSpeed * dt);
        else if (Keyboard::keyDown(Keyboard::Key::K))   camera->transform.position -= tangent * (lateralSpeed * dt);
        if      (Keyboard::keyDown(Keyboard::Key::J))   camera->transform.position -= glm::cross(normal, tangent) * (lateralSpeed * dt);
        else if (Keyboard::keyDown(Keyboard::Key::L))  camera->transform.position += glm::cross(normal, tangent) * (lateralSpeed * dt);
        
        camera->transform.position = glm::normalize(camera->transform.position()) * centerDist;
    }
}

// adjusts the camera's forward vector based on distance from the surface
void adjustCamera(Entity* camera, vec3 positionDir, float dist) {

    constexpr auto far = 2.f;
    constexpr auto near = 0.05f;

    auto forward = camera->transform.forward();
    auto tangent = getTangent(positionDir, forward);
    
    auto point = positionDir * 2.f;
    DrawDebug::getInstance().drawDebugVector(point, point + tangent, vec3(0,1,1));

    // nlerp between looking at the planet's center and the tangent direction to the surface
    auto faceDir = glm::normalize(glm::mix(-positionDir, tangent, clampf((far - dist) / (far - near), 0.f, 1.f)));
    camera->transform.rotation = quat(rotateBetween(vec3(0,0,1), faceDir));
}