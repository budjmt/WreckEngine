#include "TriPlay.h"

#include "TextEntity.h"

#include <iostream>

#include "slot_map.h"

namespace {
    void menu_update(LogicEntity* e, double dt) {
        if (Keyboard::keyPressed(Keyboard::Key::Code::Space)) {
            Event::Trigger(e).sendEvent(Event::Handler::get("menu_state"), Event::Message::get("start_game"));
        }
    }
}

static Render::Info::res_proxy<float> exposure;

TriPlay::TriPlay() : Game(6)
{
    auto menuState = make_shared<State>("menu");
    auto mainState = make_shared<State>("main");
    auto mainsp = mainState.get();

    ADD_EVENT(start_game);
    menuState->handler_func = [this, mainsp, start_game_event](Event::Handler::param_t e) {
        if (e.id == start_game_event) {
            currState = mainsp;
            vec4 clearColor = Color::fromRgb(48, 5, 56);
            Thread::Render::runNextFrame([=] { GLframebuffer::setClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a); });
        }
    };
    addState(menuState);
    menuState->addEntity(make_shared<LogicEntity>(menu_update));
    
    auto menuText = make_shared<TextEntity>("Press space to begin.", "arial.ttf", Text::Justify::MIDDLE, Text::Justify::MIDDLE, 48);
    menuText->transform.position = vec3(Window::width * 0.5f, Window::height * 0.5f, 0);
    menuText->transform.scale = vec3(0.5f, 1, 1);
    menuState->addEntity(menuText);
    //Text::active = false;

   GLframebuffer::setClearColor(0, 0.5f, 0.2f, 1);

    mainState->handler_func = [](Event::Handler::param_t e) {
        //nothing right now
    };
    addState(mainState);

    auto prog = loadProgram("Shaders/matvertexShader.glsl", "Shaders/matfragmentShader.glsl");

    auto m = loadOBJ("Assets/basic.obj");
    m->translateTo(vec3());
    auto ndm = make_shared<DrawMesh>(&renderer.deferred.objects, m, "Assets/texture.png", prog);
    auto mesh = make_shared<ColliderEntity>(ndm);
    ndm->material.addResource<GLcamera::matrix>("cameraMatrix");
    mesh->id = (void*)0xcaca;
    mainState->addEntity(mesh);
    //me = mesh;

    std::vector<std::vector<vec3>> k = {
        { vec3( 1,-1,-1), vec3( 1,-1, 1), vec3( 1, 1, 1) },
        { vec3(-1, 1,-1), vec3(-1, 1, 1), vec3(-1,-1, 1) },
        { vec3(-1,-1, 1), vec3(-1,-1,-1), vec3(-1, 1,-1) },
        { vec3( 1, 1,-1), vec3( 1, 1, 1), vec3( 1,-1, 1) }
    };
    /*std::vector<std::vector<vec3>> k = {
        { vec3( 1, 1, 0), vec3(-1, 1, 0) },
        { vec3(1,-1, 0), vec3(-1,-1, 0) }
    };*/
    //genBezierSurface("Assets/bezier.obj",16,16,k);
    //genCone("Assets/cone.obj", 8);
    //auto bezier = loadOBJ("Assets/bezier.obj");
    auto cone = loadOBJ("Assets/cone.obj");
    mesh = make_shared<ColliderEntity>(make_shared<DrawMesh>(&renderer.deferred.objects, cone, ndm->material));
    mesh->transform.position = vec3(2.5f, 0, 0);
    //mesh->id = (void*)0xb;
    mesh->id = (void*)0xc1;
    mainState->addEntity(mesh);

    //genCylinder("Assets/cylinder.obj", 64);
    auto cylinder = loadOBJ("Assets/cylinder.obj");
    mesh = make_shared<ColliderEntity>(make_shared<DrawMesh>(&renderer.deferred.objects, cylinder, ndm->material));
    mesh->id = (void*)0xc;
    mesh->transform.position = vec3(-2.5f, 0, 0);
    mainState->addEntity(mesh);
    mesh->rigidBody.floating(1);

    //genSphere("Assets/sphere.obj", 16);
    auto sphere = loadOBJ("Assets/sphere.obj");
    mesh = make_shared<ColliderEntity>(make_shared<DrawMesh>(&renderer.deferred.objects, sphere, ndm->material));
    mesh->id = (void*)0xcc;
    mesh->transform.position = vec3(0, 2.5f, 0);
    mainState->addEntity(mesh);
    mesh->rigidBody.floating(1);

    //genCube("Assets/cube.obj");
    auto cube = loadOBJ("Assets/cube.obj");
    mesh = make_shared<ColliderEntity>(make_shared<DrawMesh>(&renderer.deferred.objects, cube, ndm->material));
    mesh->id = (void*)0xc2fb;
    mesh->transform.position = vec3(0, -5.f, 0);
    mesh->transform.scale = vec3(64, 1.5f, 64);
    mesh->rigidBody.floating(1);
    mesh->rigidBody.mass(100000);
    mainState->addEntity(mesh);

    auto normalMapProg = loadProgram("Shaders/normalMapTest_v.glsl", "Shaders/normalMapTest_f.glsl");
    //auto normalMapProg = prog;

    auto cube2 = make_shared<Mesh>(*cube);
    //auto cube2 = make_shared<Mesh>(*cylinder);
    cube2->resetRenderData();
    //auto cube2 = loadOBJ("Assets/phone.obj");
    cube2->scaleTo(vec3(1.0f));
    auto dm = make_shared<DrawMesh>(&renderer.deferred.objects, cube2, "Assets/butt.png", normalMapProg, true);
    dm->material.addResource<GLcamera::matrix>("cameraMatrix");
    dm->material.addTexture(Renderable::genTexture2D("Assets/face_nm.png"));
    //dm->material.addTexture(Renderable::genTexture2D("Assets/phone_nm.png"));
    
    mesh = make_shared<ColliderEntity>(dm);
    mesh->id = (void*)0xc2;
    mesh->transform.position = vec3(-2.5f, -2.5f, 0);
    mesh->rigidBody.floating(1);
    mainState->addEntity(mesh);
    me = mesh;

    auto forwardProg = loadProgram("Shaders/forwardTest_v.glsl", "Shaders/forwardTest_f.glsl");

    dm = make_shared<DrawMesh>(&renderer.forward.objects, sphere, "Assets/butt.png", forwardProg);
    mesh = make_shared<ColliderEntity>(dm);
    renderer.lights.connectLightBlocks(forwardProg, "PointBlock", "SpotlightBlock", "DirectionalBlock");
    dm->color(vec4(0, 1, 0.5f, 0.5f));
    dm->material.addResource<GLcamera::matrix>("cameraMatrix");
    dm->material.addResource<GLcamera::position>("camPos");
    mesh->transform.position = vec3(-5.f, 0, 0);
    mesh->rigidBody.floating(1);
    mainState->addEntity(mesh);

    auto camera = make_shared<Camera>();
    camera->id = (void*)0xcab;
    camera->transform.position = vec3(0, 0, 1);
    camera->transform.rotate(0, PI, 0);
    mainState->addEntity(camera);

    setupLights();
    setupPostProcess();
}

void TriPlay::setupLights() {
    Light::Group<Light::Point> group;

    for (auto i = 0; i < 100; ++i) {
        Light::Point point;
        point.position = Random::getRange(vec3(-8.f), vec3(8.f));
        point.color = Random::getRange(vec3(0), vec3(1));
        point.falloff = vec2(0.5f, 1.f);
        group.addLight(point, Light::UpdateFreq::NEVER);
    }

    dLight.light.position = vec3();
    dLight.light.color = vec3(0,0,1);
    dLight.light.falloff = vec2(1.f, 3.f);
    dLight.light.tag = 1;
    dLight.key = group.addLight(dLight.light, Light::UpdateFreq::OFTEN);
    dLight.group = renderer.lights.pointLights.getGroup(0);

    dLight2.light.position = vec3(0,1,0);
    dLight2.light.color = vec3(1, 0, 0);
    dLight2.light.falloff = vec2(2.75f, 4.5f);
    dLight2.light.tag = 2;
    dLight2.key = group.addLight(dLight2.light, Light::UpdateFreq::SOMETIMES);
    dLight2.group = dLight.group;

    Light::Group<Light::Directional> directional;
    Light::Directional d;
    d.direction = normalize(vec3(-1,-1,-0.5f));
    d.color = vec3(1);
    directional.addLight(d, Light::UpdateFreq::NEVER);

    Light::Group<Light::Spotlight> spot;
    Light::Spotlight s;
    s.position = vec3(3, 2, 0.5f);
    s.direction = normalize(vec3(-1, -1, 0));
    s.color = vec3(1,0,0);
    s.falloffRad = vec2(1.f, 3.f);
    s.falloffLen = vec2(1.f, 20.f);
    spot.addLight(s, Light::UpdateFreq::NEVER);

    renderer.lights.pointLights.setGroups({ group });
    renderer.lights.directionalLights.setGroups({ directional });
    renderer.lights.spotLights.setGroups({ spot });
}

void TriPlay::setupPostProcess() {
    using namespace Render;

    renderer.forward.postProcess.output = Target::create<GLubyte>();

    auto& colorRender  = gBuffer[0], 
        & brightRender = gBuffer[1];

    // bright pass
    auto brightPass = make_shared<PostProcess>();
    brightPass->data.setShaders(PostProcess::make_program("Shaders/postProcess/brightPass.glsl"));
    brightPass->data.setTextures(colorRender);
    brightPass->renderToTextures(brightRender);

    // blur
    auto blurPath = "Shaders/postProcess/blur.glsl";
    auto blurH = make_shared<PostProcess>(), blurV = make_shared<PostProcess>();
    auto blurTarget = Target::create<GLubyte>();
    auto blurF = loadShader(blurPath, GL_FRAGMENT_SHADER);
    
    blurH->data.setShaders(PostProcess::make_program(blurF, blurPath));
    blurH->data.setTextures(brightRender);
    blurH->renderToTextures(blurTarget);
    blurH->data.shaders->program.use();
    blurH->data.shaders->program.setOnce<GLboolean>("horizontal", true);

    blurV->data.setShaders(PostProcess::make_program(blurF, blurPath));
    blurV->data.setTextures(blurTarget);
    blurV->renderToTextures(brightRender);
    blurV->data.shaders->program.use();
    blurV->data.shaders->program.setOnce<GLboolean>("horizontal", false);

    // bloom
    auto bloom = make_shared<PostProcess>();
    bloom->data.setShaders(PostProcess::make_program("Shaders/postProcess/bloom.glsl"));
    bloom->data.setTextures(colorRender, brightRender);
    bloom->renderToTextures(blurTarget);
    //bloom->renderToTextures(renderer.postProcess.output);
    bloom->data.shaders->program.use();
    bloom->data.setSamplers(1, "brightBlur");

    // HDR
    auto hdr = make_shared<PostProcess>();
    hdr->data.setShaders(PostProcess::make_program("Shaders/postProcess/hdr.glsl"));
    exposure = hdr->data.addResource<float>("exposure");
    exposure->value = 2;
    hdr->data.setTextures(blurTarget);
    hdr->renderToTextures(brightRender);

    // CA
    auto chromaticAberration = make_shared<PostProcess>();
    chromaticAberration->data.setShaders(PostProcess::make_program("Shaders/postProcess/CA.glsl"));
    chromaticAberration->data.setTextures(blurTarget);
    chromaticAberration->renderToTextures(colorRender);

    // CRT
    auto crt = make_shared<PostProcess>();
    crt->data.setShaders(PostProcess::make_program("Shaders/postProcess/crt.glsl"));
    crt->data.setTextures(colorRender);
    crt->renderToTextures(renderer.forward.postProcess.output);
    crt->data.addResource<GLtime>("time");
    crt->data.addResource<GLresolution>("resolution");

    renderer.forward.postProcess.entry.chainsTo(brightPass)->chainsTo(blurH)->cyclesWith(2, blurV)->chainsTo(bloom)->chainsTo(chromaticAberration)->chainsTo(crt);
}

#include "CollisionManager.h"
void TriPlay::update(double delta) {
    const auto dt = (float)delta;

    Game::update(delta);

    //quit the game
    if (Keyboard::keyDown(Keyboard::Key::Code::Q)) Window::close();

    constexpr auto speed = 5.f;

    bool shift = Keyboard::shiftDown();
    bool ctrl  = Keyboard::controlDown();

    if (exposure) {
        if (Keyboard::keyDown(Keyboard::Key::Code::RBracket)) exposure->value += 10 * dt;
        if (Keyboard::keyDown(Keyboard::Key::Code::LBracket)) exposure->value -= 10 * dt;
    }

    if (Keyboard::keyDown(Keyboard::Key::Code::I)) {
        if (shift)
            me->transform.position += vec3(0, 0, -speed * dt);
        else if (ctrl)
            me->transform.rotate(-2 * PI * dt, 0, 0);
        else
            me->transform.position += vec3(0, speed * dt, 0);
    }
    else if (Keyboard::keyDown(Keyboard::Key::Code::K)) {
        if(shift)
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

    Camera::mayaControl(Camera::main, dt);

    updateLights();

    DrawDebug::get().drawDebugVector(vec3(), vec3(1, 0, 0), vec3(1, 0, 0));
    DrawDebug::get().drawDebugVector(vec3(), vec3(0, 1, 0), vec3(0, 0, 1));
    DrawDebug::get().drawDebugVector(vec3(), vec3(0, 0, 1), vec3(0, 1, 0));
}

void TriPlay::updateLights() {
    static float mul = 0.05f;
    static uint32_t frameCounter = 1;

    dLight.light.position += vec3(mul);
    if (abs(dLight.light.position.x) > 3.f) mul = -mul;
    Thread::Render::runNextFrame([this] { dLight.group->updateLight(dLight.key, Light::UpdateFreq::OFTEN, dLight.light); });

    if (frameCounter % 30 == 0) {
        dLight2.light.color = vec3(1) - dLight2.light.color;
        Thread::Render::runNextFrame([this] { dLight2.group->updateLight(dLight2.key, Light::UpdateFreq::SOMETIMES, dLight2.light); });
    }
    ++frameCounter;
}

void TriPlay::postUpdate() {
    Game::postUpdate();
    Text::postUpdate();
}

void TriPlay::physicsUpdate(double dt) {
    Game::physicsUpdate(dt);
    CollisionManager::getInstance().update((float)dt);
}

void TriPlay::draw() {
    Game::draw();
    Text::render(&renderer.forward.objects);
}