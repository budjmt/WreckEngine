#include "TessellatorTest.h"

#include "ColliderEntity.h"
#include "HotSwap.h"
#include "TextEntity.h"

shared<TextEntity> controlText;
shared<ColliderEntity> me, plane;

TessellatorTest::TessellatorTest() : Game(6) {
    auto mainState = make_shared<State>("main");
    addState(mainState);

    controlText = make_shared<TextEntity>("", "arial.ttf", Text::Justify::MIDDLE, Text::Justify::START, 48);
    controlText->transform.position = vec3(25, 50, 0);
    controlText->transform.scale = vec3(0.5f, 1, 1);
    mainState->addEntity(controlText);

    auto tessProg = HotSwap::Shader::create();
    using ShaderRes = decltype(tessProg->vertex);

    tessProg->vertex = ShaderRes("Shaders/planet_v.glsl", GL_VERTEX_SHADER);
    tessProg->tessControl = ShaderRes("Shaders/planet_tc.glsl", GL_TESS_CONTROL_SHADER);
    tessProg->tessEval = ShaderRes("Shaders/planet_te.glsl", GL_TESS_EVALUATION_SHADER);
    tessProg->fragment = ShaderRes("Shaders/planet_f.glsl", GL_FRAGMENT_SHADER);
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

    auto planet = make_shared<ColliderEntity>(dm);
    planet->id = (void*)0xabc;
    planet->rigidBody.floating(1.f);
    //mainState->addEntity(planet);

    controlData.prog = loadProgram("Shaders/matvertexShader.glsl", "Shaders/dumb_f.glsl");
    controlData.mat = controlData.prog.getUniform<mat4>("cameraMatrix");

    //genPlane("Assets/plane.obj", 5);
    auto planeMesh = loadOBJ("Assets/plane.obj");
    dm = make_shared<DrawMesh>(&renderer.forward.objects, planeMesh, "Assets/texture.png", controlData.prog);
    dm->renderGroup = 1;

    plane = make_shared<ColliderEntity>(dm);
    plane->rigidBody.floating(1.f);
    mainState->addEntity(plane);

    me = make_shared<ColliderEntity>(make_shared<DrawMesh>(&renderer.forward.objects, cube, "Assets/texture.png", controlData.prog));
    me->transform.position = vec3(-3, 0, 0);
    me->rigidBody.floating(1.f);
    me->color = vec4(1, 0, 0, 1);
    mainState->addEntity(me);

    auto camera = make_shared<Camera>();
    camera->id = (void*)0xcab;
    camera->transform.position = vec3(0, 0, 3);
    camera->transform.rotate(0, PI, 0);
    mainState->addEntity(camera);

    renderer.lightingOn = false;

    DrawDebug::getInstance().flush();
    Text::flush();
}

vec3 getClosestSphereDir(vec3, float, vec3, vec3);

void TessellatorTest::update(double delta) {
    const auto dt = (float)delta;

    Game::update(delta);

    //quit the game
    if (Keyboard::keyDown(Keyboard::Key::Code::Q)) Window::close();

    auto n = getClosestSphereDir(vec3(), 2, Camera::main->transform.position, Camera::main->transform.forward());
    //printf("%s\n", to_string(n).c_str());
    auto p = vec3() + 2.f * n;
    plane->transform.position = p;
    plane->transform.rotation = quat(rotateBetween(vec3(0, 0, 1), n));

    constexpr auto speed = 5.f;
    bool shift = Keyboard::shiftDown();
    bool ctrl = Keyboard::controlDown();

    if (Keyboard::keyDown(Keyboard::Key::Code::I)) {
        if (shift)
            me->transform.position += vec3(0, 0, -speed * dt);
        else if (ctrl)
            me->transform.rotate(-2 * PI * dt, 0, 0);
        else
            me->transform.position += vec3(0, speed * dt, 0);
    }
    else if (Keyboard::keyDown(Keyboard::Key::Code::K)) {
        if (shift)
            me->transform.position += vec3(0, 0, speed * dt);
        else if (ctrl)
            me->transform.rotate(2 * PI * dt, 0, 0);
        else
            me->transform.position += vec3(0, -speed * dt, 0);
    }
    if (Keyboard::keyDown(Keyboard::Key::Code::L)) {
        if (shift)
            me->transform.rotate(0, 2 * PI * dt, 0);
        else if (ctrl)
            me->transform.rotate(0, 0, 2 * PI * dt);
        else
            me->transform.position += vec3(speed * dt, 0, 0);
    }
    else if (Keyboard::keyDown(Keyboard::Key::Code::J)) {
        if (shift)
            me->transform.rotate(0, -2 * PI * dt, 0);
        else if (ctrl)
            me->transform.rotate(0, 0, -2 * PI * dt);
        else
            me->transform.position += vec3(-speed * dt, 0, 0);
    }

    Camera::mayaCam(Camera::main, dt);
    controlText->setMessage(to_string(me->transform.getComputed()->position, 3) + "\n" + std::to_string(glm::length(me->transform.getComputed()->position())));
}

void TessellatorTest::postUpdate() {
    Game::postUpdate();
    Text::postUpdate();
}

void TessellatorTest::draw() {
    auto mat = Camera::main->getCamMat();
    planetData.prog.use();
    planetData.mat.update(mat);
    //planetData.pos.update(Camera::main->transform.getComputed()->position);
    planetData.pos.update(me->transform.getComputed()->position);
    controlData.prog.use();
    controlData.mat.update(mat);

    Game::draw();
    Text::render(&renderer.forward.objects);
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