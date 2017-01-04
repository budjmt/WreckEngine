#include "TriPlay.h"

#include "TextEntity.h"

#include <iostream>

namespace {
    void menu_update(LogicEntity* e, double dt) {
        if (Keyboard::keyPressed(Keyboard::Key::Space)) {
            Event::Trigger(e).sendEvent(Event::Handler::get("menu_state"), Event::Message::get("start_game"));
        }
    }
}

TriPlay::TriPlay(GLprogram prog) : Game(6)
{
    auto menuState = make_shared<State>("menu");
    auto mainState = make_shared<State>("main");
    auto mainsp = mainState.get();

    ADD_EVENT(start_game);
    menuState->handler_func = [this, mainsp, start_game_event](Event::Handler::param_t e) {
        if (e.id == start_game_event) {
            currState = mainsp;
            GLframebuffer::setClearColor(0, 0, 0, 1);
        }
    };
    addState(menuState);
    menuState->addEntity(make_shared<LogicEntity>(menu_update));
    
    auto menuText = make_shared<TextEntity>("Press space to begin.", "QuartzMS.ttf", Text::Justify::MIDDLE, Text::Justify::MIDDLE, 48);
    menuText->transform.position = vec3(Window::width * 0.5f, Window::height * 0.5f, 0);
    menuText->transform.scale = vec3(0.5f, 1, 1);
    menuState->addEntity(menuText);
    //Text::active = false;

    GLframebuffer::setClearColor(0, 0.5f, 0.2f, 1);

    mainState->handler_func = [](Event::Handler::param_t e) {
        //nothing right now
    };
    addState(mainState);

    auto m = loadOBJ("Assets/basic.obj");
    m->translateTo(vec3());
    auto mesh = make_shared<ColliderEntity>(make_shared<DrawMesh>(&renderer.opaque.objects, m, "Assets/texture.png", prog));
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
    mesh = make_shared<ColliderEntity>(make_shared<DrawMesh>(&renderer.opaque.objects, cone, "Assets/texture.png", prog));
    mesh->transform.position = vec3(2.5f, 0, 0);
    //mesh->id = (void*)0xb;
    mesh->id = (void*)0xc1;
    mainState->addEntity(mesh);

    //genCylinder("Assets/cylinder.obj", 64);
    auto cylinder = loadOBJ("Assets/cylinder.obj");
    mesh = make_shared<ColliderEntity>(make_shared<DrawMesh>(&renderer.opaque.objects, cylinder, "Assets/texture.png", prog));
    mesh->id = (void*)0xc;
    mesh->transform.position = vec3(-2.5f, 0, 0);
    mainState->addEntity(mesh);
    mesh->rigidBody.floating(1);

    //genSphere("Assets/sphere.obj", 16);
    auto sphere = loadOBJ("Assets/sphere.obj");
    mesh = make_shared<ColliderEntity>(make_shared<DrawMesh>(&renderer.opaque.objects, sphere, "Assets/texture.png", prog));
    mesh->id = (void*)0xcc;
    mesh->transform.position = vec3(0, 2.5f, 0);
    mainState->addEntity(mesh);
    mesh->rigidBody.floating(1);

    //genCube("Assets/cube.obj");
    auto cube = loadOBJ("Assets/cube.obj");
    mesh = make_shared<ColliderEntity>(make_shared<DrawMesh>(&renderer.opaque.objects, cube, "Assets/texture.png", prog));
    mesh->id = (void*)0xc2fb;
    mesh->transform.position = vec3(0, -5.f, 0);
    mesh->transform.scale = vec3(64, 1.5f, 64);
    mesh->rigidBody.floating(1);
    mesh->rigidBody.mass(100000);
    mainState->addEntity(mesh);

    mesh = make_shared<ColliderEntity>(make_shared<DrawMesh>(&renderer.opaque.objects, cube, "Assets/butt.png", prog));
    mesh->id = (void*)0xc2;
    mesh->transform.position = vec3(-2.5f, -2.5f, 0);
    mesh->rigidBody.floating(1);
    mainState->addEntity(mesh);
    me = mesh;

    auto forwardProg = loadProgram("Shaders/forwardTest_v.glsl", "Shaders/forwardTest_f.glsl");
    mesh = make_shared<ColliderEntity>(make_shared<DrawMesh>(&renderer.alpha.objects, sphere, "Assets/butt.png", forwardProg));
    renderer.lights.connectLightBlocks(forwardProg, "PointBlock", "SpotlightBlock", "DirectionalBlock");
    mesh->color = vec4(0, 1, 0.5f, 0.5f);
    mesh->transform.position = vec3(-5.f, 0, 0);
    mesh->rigidBody.floating(1);
    mainState->addEntity(mesh);

    forwardData.prog = forwardProg;
    forwardData.mat = forwardProg.getUniform<mat4>("cameraMatrix");
    forwardData.pos = forwardProg.getUniform<vec3>("camPos");

    auto camera = make_shared<Camera>(prog);
    camera->id = (void*)0xcab;
    camera->transform.position = vec3(0, 0, 1);
    camera->transform.rotate(0, PI, 0);
    mainState->addEntity(camera);

    objectData.prog = prog;
    objectData.mat = prog.getUniform<mat4>("cameraMatrix");

    if(DEBUG) DrawDebug::getInstance().camera(camera.get());

    setupLights();
    setupPostProcess();
}

void TriPlay::setupLights() {
    Light::Group<Light::Point> group;

    for (auto i = 0; i < 100; ++i) {
        Light::Point point;
        point.position = vec3(Random::getRange(0, 16) - 8, Random::getRange(0, 16) - 8, Random::getRange(0, 16) - 8);
        point.color = vec3(Random::getf(), Random::getf(), Random::getf());
        point.falloff = vec2(0.5f, 1.f);
        group.addLight(point, Light::UpdateFreq::NEVER);
    }

    dLight.light.position = vec3();
    dLight.light.color = vec3(0,0,1);
    dLight.light.falloff = vec2(1.f, 3.f);
    dLight.light.tag = 1;
    group.addLight(dLight.light, Light::UpdateFreq::OFTEN);

    dLight2.light.position = vec3(0,1,0);
    dLight2.light.color = vec3(1, 0, 0);
    dLight2.light.falloff = vec2(2.75f, 4.5f);
    dLight2.light.tag = 2;
    group.addLight(dLight2.light, Light::UpdateFreq::SOMETIMES);

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

    dLight.index  = group.getLightIndexByTag(1, Light::UpdateFreq::OFTEN);
    dLight2.index = group.getLightIndexByTag(2, Light::UpdateFreq::SOMETIMES);
    dLight.group  = &renderer.lights.pointLights.getGroup(0);
    dLight2.group = dLight.group;
}

void TriPlay::setupPostProcess() {
    using namespace Render;

    renderer.alpha.postProcess.output = GLframebuffer::createRenderTarget<GLubyte>();

    auto& colorRender  = gBuffer[0], 
        & brightRender = gBuffer[1];

    // bright pass
    auto brightPass = make_shared<PostProcess>();
    brightPass->data.setShaders(PostProcess::make_program("Shaders/postProcess/brightPass.glsl"));
    brightPass->data.setTextures(colorRender);
    brightPass->renderToTextures(brightRender);

    // blur
    auto blurH = make_shared<PostProcess>(), blurV = make_shared<PostProcess>();
    auto blurTarget = GLframebuffer::createRenderTarget<GLubyte>();
    auto blurF = loadShader("Shaders/postProcess/blur.glsl", GL_FRAGMENT_SHADER);
    
    blurH->data.setShaders(PostProcess::make_program(blurF));
    blurH->data.setTextures(brightRender);
    blurH->renderToTextures(blurTarget);
    blurH->data.shaders->program.use();
    blurH->data.shaders->program.setOnce<GLboolean>("horizontal", true);

    blurV->data.setShaders(PostProcess::make_program(blurF));
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

    // CA
    auto chromaticAberration = make_shared<PostProcess>();
    chromaticAberration->data.setShaders(PostProcess::make_program("Shaders/postProcess/CA.glsl"));
    chromaticAberration->data.setTextures(blurTarget);
    chromaticAberration->renderToTextures(colorRender);

    // CRT
    auto crt = make_shared<PostProcess>();
    crt->data.setShaders(PostProcess::make_program("Shaders/postProcess/crt.glsl"), &crtTime, &crtRes);
    crt->data.setTextures(colorRender);
    crt->renderToTextures(renderer.alpha.postProcess.output);
    crtTime = GLresource<GLtime>(crt->data.shaders->program, "time");
    crtRes  = GLresource<GLresolution>(crt->data.shaders->program, "resolution");

    renderer.alpha.postProcess.entry.chainsTo(brightPass)->chainsTo(blurH)->cyclesWith(2, blurV)->chainsTo(bloom)->chainsTo(chromaticAberration)->chainsTo(crt);
}

#include "CollisionManager.h"
void TriPlay::update(double delta) {
    const auto dt = (float)delta;

    Game::update(delta);
    CollisionManager::getInstance().update(dt);

    //quit the game
    if (Keyboard::keyDown(Keyboard::Key::Code::Q)) exit('q');

    constexpr auto speed = 5.f;

    bool shift = Keyboard::shiftDown();
    bool ctrl  = Keyboard::controlDown();

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

    Camera::mayaCam(Camera::main, dt);
    
    auto mat = Camera::main->getCamMat();
    objectData.prog.use();
    objectData.mat.update(mat);
    forwardData.prog.use();
    forwardData.mat.update(mat);
    forwardData.pos.update(Camera::main->transform.getComputed()->position);

    DrawDebug::getInstance().drawDebugVector(vec3(), vec3(1, 0, 0), vec3(1, 0, 0));
    DrawDebug::getInstance().drawDebugVector(vec3(), vec3(0, 1, 0), vec3(0, 0, 1));
    DrawDebug::getInstance().drawDebugVector(vec3(), vec3(0, 0, 1), vec3(0, 1, 0));

    updateLights();
}

void TriPlay::updateLights() {
    static float mul = 0.05f;
    static uint32_t frameCounter = 1;

    dLight.light.position += vec3(mul);
    if (abs(dLight.light.position.x) > 3.f) mul = -mul;
    dLight.group->updateLight(dLight.index, Light::UpdateFreq::OFTEN, dLight.light);

    if (frameCounter % 30 == 0) {
        dLight2.light.color = vec3(1) - dLight2.light.color;
        dLight2.group->updateLight(dLight2.index, Light::UpdateFreq::SOMETIMES, dLight2.light);
    }
    ++frameCounter;
}

void TriPlay::draw() {
    Game::draw();
    Text::render(&renderer.alpha.objects);
}