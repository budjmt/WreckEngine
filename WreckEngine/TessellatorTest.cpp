#include "TessellatorTest.h"

#include "ColliderEntity.h"
#include "HotSwap.h"
#include "TextEntity.h"
#include "ComputeEntity.h"

struct RenderData {
    GLprogram prog;
    GLuniform<mat4> mat;
    GLuniform<vec3> pos;
};
static RenderData planetData, controlData;

struct {
    GLprogram prog;
    GLuniform<float> time, zoom;
    GLtexture cubemap;
} cubemapData;

static shared<TextEntity> controlText;
static shared<Entity> me, planet, plane, cameraControl;

struct {
    vec3 forward;
} cameraNav;

void initCubemap(GLtexture& tex, GLenum type, GLuint width, GLuint height, GLenum from, GLenum to) {
    tex.create(GL_TEXTURE_CUBE_MAP);
    tex.bind();
    tex.param(GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    tex.param(GL_TEXTURE_MAG_FILTER, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    tex.set2DAs(GL_TEXTURE_CUBE_MAP_POSITIVE_X, type, nullptr, width, height, from, to);
    tex.set2DAs(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, type, nullptr, width, height, from, to);
    tex.set2DAs(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, type, nullptr, width, height, from, to);
    tex.set2DAs(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, type, nullptr, width, height, from, to);
    tex.set2DAs(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, type, nullptr, width, height, from, to);
    tex.set2DAs(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, type, nullptr, width, height, from, to);
}

TessellatorTest::TessellatorTest() : Game(6) {
    auto mainState = make_shared<State>("main");
    addState(mainState);

    controlText = make_shared<TextEntity>("", "arial.ttf", Text::Justify::MIDDLE, Text::Justify::START, 48);
    controlText->transform.position = vec3(25, 50, 0);
    controlText->transform.scale = vec3(0.5f, 1, 1);
    mainState->addEntity(controlText);
    
    auto cubemapProg = HotSwap::Shader::create();
    using ShaderRes = decltype(cubemapProg->vertex);
    
    cubemapProg->compute = ShaderRes("Shaders/cubemap_c.glsl", GL_COMPUTE_SHADER);
    cubemapProg->setupProgram();
    cubemapData.prog = cubemapProg->program();
    cubemapData.prog.use();
    cubemapData.time = cubemapData.prog.getUniform<float>("Time");
    cubemapData.zoom = cubemapData.prog.getUniform<float>("Zoom");
    cubemapData.zoom.update(6.0f);

    constexpr size_t texSize = 256;
    initCubemap(cubemapData.cubemap, GL_FLOAT, texSize, texSize, GL_RGBA, GL_RGBA32F);

    auto computeEntity = make_shared<ComputeTextureEntity>();
    computeEntity->program = cubemapData.prog;
    computeEntity->dispatchSize = { texSize, texSize, 6 };
    computeEntity->update_uniforms = [&]() {
        cubemapData.time.update((float)Time::elapsed());
        //cubemapData.zoom.update(cosRange(time * 0.375f, 1, 8));
    };
    computeEntity->texture = cubemapData.cubemap;
    computeEntity->synchronize = false;
    mainState->addEntity(computeEntity);

    checkProgLinkError(cubemapData.prog);

    auto tessProg = HotSwap::Shader::create();
    tessProg->vertex      = ShaderRes("Shaders/planet_v.glsl",  GL_VERTEX_SHADER);
    tessProg->tessControl = ShaderRes("Shaders/planet_tc.glsl", GL_TESS_CONTROL_SHADER);
    tessProg->tessEval    = ShaderRes("Shaders/planet_te.glsl", GL_TESS_EVALUATION_SHADER);
    tessProg->fragment    = ShaderRes("Shaders/planet_f.glsl",  GL_FRAGMENT_SHADER);
    tessProg->setupProgram();

    planetData.prog = tessProg->program();
    checkProgLinkError(planetData.prog);

    planetData.prog.use();
    planetData.mat = planetData.prog.getUniform<mat4>("cameraMatrix");
    planetData.pos = planetData.prog.getUniform<vec3>("camPos");

    auto cube = loadOBJ("Assets/cube.obj");

    auto dm = make_shared<DrawMesh>(&renderer.forward.objects, cube, "Assets/texture.png", planetData.prog);
    dm->tesselPrim = GL_PATCHES;
    dm->renderGroup = renderer.forward.objects.addGroup([] {
        GL_CHECK(glEnable(GL_CULL_FACE));
        //GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
        GLsynchro::barrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }, [] {
        GL_CHECK(glDisable(GL_CULL_FACE));
        //GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
    });
    dm->material.addTexture(cubemapData.cubemap);

    planet = make_shared<Entity>(dm);
    planet->id = (void*)0xabc;
    mainState->addEntity(planet);

    //genPlane("Assets/plane.obj", 5);
    dm = make_shared<DrawMesh>(&renderer.forward.objects, loadOBJ("Assets/plane.obj"), "Assets/texture.png", planetData.prog);
    dm->tesselPrim = GL_PATCHES;
    dm->renderGroup = 1;
    dm->material.addTexture(cubemapData.cubemap);

    plane = make_shared<Entity>(dm);
    plane->active = false;
    mainState->addEntity(plane);

    cameraControl = make_shared<TransformEntity>();
    cameraControl->transform.position = vec3(0, 0, 5);
    cameraControl->transform.rotate(0, PI, 0);
    //cameraControl->transform.position = vec3(-3, 0, 0);
    mainState->addEntity(cameraControl);

    auto camera = make_shared<Camera>();
    camera->id = (void*)0xcab;
    //camera->transform.position = vec3(0, 0, 5);
    //camera->transform.rotate(0, PI, 0);
    camera->transform.parent(&cameraControl->transform);
    mainState->addEntity(camera);

    controlData.prog = loadProgram("Shaders/matvertexShader.glsl", "Shaders/dumb_f.glsl");
    controlData.mat = controlData.prog.getUniform<mat4>("cameraMatrix");

    auto cone = loadOBJ("Assets/cone.obj");
    cone->rotate(quat::rotation(PI / 2, vec3(1, 0, 0)));
    me = make_shared<Entity>(make_shared<DrawMesh>(&renderer.forward.objects, cone, "Assets/texture.png", controlData.prog));
    me->transform.position = vec3(-3, 0, 0);
    //me->transform.parent(&cameraControl->transform);
    me->color = vec4(1,0,0,1);
    mainState->addEntity(me);

    cameraNav.forward = camera->forward();

    renderer.lightingOn = false;
    
    if (DEBUG) DrawDebug::getInstance().camera(camera.get());

}

vec3 getClosestSphereDir(vec3, float, vec3, vec3);
void moveCamera(Entity* cameraControl, Entity* camera, float radius);
void adjustCamera(Entity* camera, float dist);

void TessellatorTest::update(double delta) {
    const auto dt = (float)delta;

    Game::update(delta);

    //quit the game
    //if (Keyboard::keyDown(Keyboard::Key::Code::Q)) Window::close(); // pretty broken in this game

    DrawDebug::getInstance().drawDebugVector(vec3(), vec3(1, 0, 0), vec3(1, 0, 0));
    DrawDebug::getInstance().drawDebugVector(vec3(), vec3(0, 1, 0), vec3(0, 1, 0));
    DrawDebug::getInstance().drawDebugVector(vec3(), vec3(0, 0, 1), vec3(0, 0, 1));

    const float radius = 2;

    //auto cam = me.get();
    auto cam = Camera::main;

    moveCamera(cameraControl.get(), cam, radius);

    auto pos = Camera::main->transform.getComputed()->position();
    auto dist = glm::length(pos) - radius;
    if (dist < 2) {
        plane->active = true;
        planet->active = false;

        auto n = getClosestSphereDir(vec3(), radius, pos, Camera::main->transform.getComputed()->forward());
        auto closestToForward = radius * n;
        plane->transform.position = closestToForward;
        
        plane->transform.rotation = quat(rotateBetween(vec3(0, 0, 1), n));
        
        auto forward = plane->transform.forward();
        auto camUp   = Camera::main->transform.getComputed()->up();
        auto correctUp = vec3(-forward.x * forward.y, forward.x * forward.x + forward.z * forward.z, -forward.z * forward.y);
        //auto correctUp = camUp - forward * (glm::dot(camUp, forward));
        plane->transform.rotation *= quat(rotateBetween(plane->transform.up(), correctUp));

        float scaleFactor;
        if      (dist < 0.25f) scaleFactor = 1.5;
        else if (dist < 0.75f) scaleFactor = 3.5;
        else                   scaleFactor = 7.5;

        plane->transform.scale = vec3(scaleFactor);
    }
    else {
        planet->active = true;
        plane->active = false;
    }

    pos = cam->transform.getComputed()->position();
    controlText->setMessage(to_string(pos, 3) + "\n" + std::to_string(glm::length(pos)) + "\n" + to_string(quat::getEuler(cam->transform.getComputed()->rotation())));
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
    //planeData.prog.use();
    //planeData.mat.update(mat);

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

void moveCamera(Entity* cameraControl, Entity* camera, float radius) {
    auto dt = (float)Time::delta;

    // Need another layer of transform parenting so the camera can rotate independently

    //auto mouse = Mouse::info;
    //if (mouse.down) {
    //    // mouse coords are represented in screen coords
    //    auto dx = (float)(mouse.curr.x - mouse.prev.x);
    //    auto dy = (float)(mouse.curr.y - mouse.prev.y);
    //
    //    if (mouse.getButtonState(GLFW_MOUSE_BUTTON_LEFT)) {
    //
    //        dx *= 10.f * dt;
    //        dy *= 10.f * dt;
    //
    //        dx = 2 * asin(dx * 0.5f);
    //        dy = 2 * asin(dy * 0.5f);
    //
    //        auto look = camera->transform.position() + camera->transform.forward();
    //        camera->transform.rotate(dy, dx, 0);
    //        camera->transform.position = look - camera->transform.getComputed()->forward();
    //    }
    //}

    auto pos = cameraControl->transform.position();
    auto centerDist = glm::length(pos);

    const auto towardSpeed = (centerDist - radius) / centerDist;

    const bool shift = Keyboard::shiftDown();
    
    if (shift) {
        // move away from/towards the surface
        if      (Keyboard::keyDown(Keyboard::Key::W)) cameraControl->transform.position -= pos * (towardSpeed * dt);
        else if (Keyboard::keyDown(Keyboard::Key::S)) cameraControl->transform.position += pos * (towardSpeed * dt);

        pos = camera->transform.getComputed()->position();
        auto dist = glm::length(pos) - radius;
        adjustCamera(camera, dist);
    }
    else {
        // move around the surface
        const auto lateralSpeed = 5.f * towardSpeed;

        float dH = 0, dV = 0;
        bool moved = false;

        if      (Keyboard::keyDown(Keyboard::Key::W)) { dV =  lateralSpeed * dt; moved = true; }
        else if (Keyboard::keyDown(Keyboard::Key::S)) { dV = -lateralSpeed * dt; moved = true; }
        if      (Keyboard::keyDown(Keyboard::Key::A)) { dH = -lateralSpeed * dt; moved = true; }
        else if (Keyboard::keyDown(Keyboard::Key::D)) { dH =  lateralSpeed * dt; moved = true; }
        
        const auto getRot = [](float d, float radius) {
            return 2 * asin(d * 0.5f / radius);
        };
       
        if (moved) {
            cameraControl->transform.rotate(getRot(dV, centerDist), getRot(dH, centerDist), 0);
            cameraControl->transform.position = cameraControl->transform.forward() * -centerDist;
        }
    }
}

// adjusts the camera's forward vector based on distance from the surface
void adjustCamera(Entity* camera, float dist) {
    
    constexpr auto farCamDist = 2.f;
    constexpr auto nearCamDist = 0.05f;

    // lerp between no rotation and orthogonal rotation; this translates to facing the surface to facing tangent to the surface when combined with the parent
    auto faceRot = glm::mix(0.f, PI * 0.5f, clampf((farCamDist - dist) / (farCamDist - nearCamDist), 0.f, 1.f));
    camera->transform.rotation = quat::rotation(faceRot, vec3(-1, 0, 0));
}