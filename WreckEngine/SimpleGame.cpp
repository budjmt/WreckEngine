#include "SimpleGame.h"

SimpleGame::SimpleGame(GLprogram prog) : Game(6) {
    auto mainState = make_shared<State>("main");
    addState(mainState);

    GLframebuffer::setClearColor(100.f / 255.f, 149.f / 255.f, 237.f / 255.f, 1);

    auto m = loadOBJ("Assets/cube.obj");
    auto ndm = make_shared<DrawMesh>(&renderer.deferred.objects, m, "Assets/snow.jpg", prog);
    auto mesh = make_shared<Entity>(ndm);
    ndm->material.addResource<GLcamera::matrix>("cameraMatrix");
    mainState->addEntity(mesh);
    me = mesh;

    m = loadOBJ("Assets/basic.obj");
    m->translateTo(vec3());
    ndm = make_shared<DrawMesh>(&renderer.deferred.objects, m, "Assets/texture.png", prog);
    mesh = make_shared<Entity>(ndm);
    ndm->material.addResource<GLcamera::matrix>("cameraMatrix");
    mesh->id = (void*)0xcaca;
    mesh->transform.position = { 1, 0, 0 };
    mainState->addEntity(mesh);

    auto camera = make_shared<Camera>();
    camera->id = (void*)0xcab;
    camera->transform.position = vec3(0, 0, 1);
    camera->transform.rotate(0, PI, 0);
    mainState->addEntity(camera);

    if (DEBUG) DrawDebug::getInstance().camera(camera.get());

    Light::Group<Light::Directional> directional;
    Light::Directional d;
    d.direction = normalize(vec3(-1, -1, -0.5f));
    d.color = vec3(1);
    directional.addLight(d, Light::UpdateFreq::NEVER);

    renderer.lights.directionalLights.setGroups({ directional });
}

void SimpleGame::update(double dt) {
    Game::update(dt);
    Camera::mayaControl(Camera::main, dt);
}