#include "TessellatorTest.h"

#include "ColliderEntity.h"
#include "HotSwap.h"
#include "TextEntity.h"
#include "ComputeEntity.h"

struct RenderData {
    GLprogram prog;
    GLuniform<mat4> mat;
    GLuniform<vec3> pos;
    GLuniform<float> radius;
};
static RenderData planetData;

struct {
    GLprogram prog;
    GLresource<float> zoom;
    GLtexture cubemap;
} noiseData;

struct {
    GLprogram prog;
    GLresource<vec3, true> camPos;
    GLresource<float> radius;
    GLtexture cubemap;
} normalData;

static shared<TextEntity> controlText;
static shared<Entity> cameraControl;

static shared<ComputeTextureEntity> noiseEntity, normalEntity;

struct TerrainPlane {
    vec3 center;
    float radius;
    vec3 boundingPoint; // for horizon culling
    Entity* entity;

    TerrainPlane(Entity* e, const Mesh* mesh, const vec3 dir, const float radius) : entity(e) {
        const auto& trans = e->transform;

        auto& verts = mesh->data().verts;
        std::vector<vec3> normalizedVerts(verts.size());
        std::transform(verts.begin(), verts.end(), normalizedVerts.begin(), [&trans](const auto& v) {
            return glm::normalize(trans.getTransformed(v));
        });

        this->center = Mesh::getCentroid(normalizedVerts) * radius;

        const auto dims = Mesh::getPreciseDims(normalizedVerts);
        this->radius = maxf(maxf(dims.x, dims.y), dims.z) * radius * 0.5f;

        float maxLen = 0;
        for (const auto& vn : normalizedVerts) {
            const auto tLen = 1.f / glm::dot(vn, dir);
            if (tLen > maxLen) {
                maxLen = tLen;
            }
        }

        const auto adjustedLen = maxLen * radius; // might need to compensate for height map
        this->boundingPoint = dir * adjustedLen;
    }
};
std::vector<TerrainPlane> planes;

struct {
    vec3 forward;
} cameraNav;

bool wireframe = false;
constexpr float RADIUS = 2;

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

shared<Entity> genPlanetPlane(const vec3 dir, const float radius, Render::MaterialPass* renderer, const size_t group) {
    static auto planeMesh = loadOBJ("Assets/plane.obj");

    auto dm = make_shared<DrawMesh>(renderer, planeMesh, "Assets/texture.png", planetData.prog);
    dm->tesselPrim = GL_PATCHES;
    dm->renderGroup = group;
    dm->material.addTexture(noiseData.cubemap);
    dm->material.addTexture(normalData.cubemap);

    auto plane = make_shared<Entity>(dm);
    plane->active = false;
    plane->transform.position = dir * radius;
    plane->transform.rotation = quat(rotateBetween(vec3(0, 0, 1), dir));
    const auto scale = radius * 2.02f;
    plane->transform.scale = vec3(scale, scale, 1);
    planes.push_back(TerrainPlane(plane.get(), planeMesh.get(), dir, radius));
    return plane;
}

TessellatorTest::TessellatorTest() : Game(6) {
    auto mainState = make_shared<State>("main");
    addState(mainState);

    controlText = make_shared<TextEntity>("", "arial.ttf", Text::Justify::MIDDLE, Text::Justify::START, 48);
    controlText->transform.position = vec3(25, 50, 0);
    controlText->transform.scale = vec3(0.5f, 1, 1);
    mainState->addEntity(controlText);

    auto cubemapProg = HotSwap::Shader::create([&] {
        noiseEntity->draw();
        normalEntity->draw();
    });
    using ShaderRes = decltype(cubemapProg->vertex);

    cubemapProg->compute = ShaderRes("Shaders/noisemap_c.glsl", GL_COMPUTE_SHADER);
    cubemapProg->setupProgram();
    noiseData.prog = cubemapProg->program();
    noiseData.prog.use();
    noiseData.zoom = noiseData.prog.getUniform<float>("Zoom");
    noiseData.zoom.value = 6.0f;

    constexpr size_t texSize = 1024;
    initCubemap(noiseData.cubemap, GL_FLOAT, texSize, texSize, GL_RGBA, GL_RGBA32F);

    auto computeDispatcher = make_shared<GraphicsWorker>();
    computeDispatcher->material.setShaders(noiseData.prog, &noiseData.zoom);
    computeDispatcher->material.setTextures();

    noiseEntity = make_shared<ComputeTextureEntity>(computeDispatcher);
    noiseEntity->dispatchSize = { texSize, texSize, 6 };
    noiseEntity->texture = noiseData.cubemap;
    noiseEntity->index = 0;
    //noiseEntity->synchronize = false;
    //mainState->addEntity(noiseEntity);
    noiseEntity->updateFreq = 0.f;
    //noiseEntity->draw();

    checkProgLinkError(noiseData.prog);

    auto normalmapProg = HotSwap::Shader::create([&] {
        normalEntity->draw();
    });
    normalmapProg->compute = ShaderRes("Shaders/normalmap_c.glsl", GL_COMPUTE_SHADER);
    normalmapProg->setupProgram();
    normalData.prog = normalmapProg->program();
    normalData.prog.use();
    normalData.camPos = GLresource<vec3, true>(normalData.prog, "CameraPosition", [&] {
        return Camera::main->transform.getComputed()->position();
    });
    normalData.radius = normalData.prog.getUniform<float>("Radius");
    normalData.radius.value = RADIUS;

    initCubemap(normalData.cubemap, GL_FLOAT, texSize, texSize, GL_RGBA, GL_RGBA32F);

    computeDispatcher = make_shared<GraphicsWorker>();
    computeDispatcher->material.setShaders(normalData.prog, &normalData.camPos, &normalData.radius);
    computeDispatcher->material.setTextures(noiseData.cubemap);

    normalEntity = make_shared<ComputeTextureEntity>(computeDispatcher);
    normalEntity->dispatchSize = { texSize, texSize, 6 };
    normalEntity->texture = normalData.cubemap;
    normalEntity->index = 0;
    normalEntity->updateFreq = 0.f;
    //normalEntity->synchronize = false;
    //mainState->addEntity(normalEntity);
    //normalEntity->draw();

    checkProgLinkError(normalData.prog);

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
    planetData.radius = planetData.prog.getUniform<float>("Radius");

    //genPlane("Assets/plane.obj", 5);
    auto* rendererUsed = &renderer.deferred.objects;
    const auto planetGroup = rendererUsed->addGroup([] {
        GL_CHECK(glEnable(GL_CULL_FACE));
        if(wireframe) GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
        GLsynchro::barrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }, [] {
        GL_CHECK(glDisable(GL_CULL_FACE));
        if(wireframe) GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
    });

    const vec3 cubeDirs[] = { vec3(0, 0, 1), vec3(0, 0, -1)
                            , vec3(0, 1, 0), vec3(0, -1, 0)
                            , vec3(1, 0, 0), vec3(-1, 0, 0) };
    for(auto& dir : cubeDirs)
        mainState->addEntity(genPlanetPlane(dir, RADIUS, rendererUsed, planetGroup));

    cameraControl = make_shared<TransformEntity>();
    cameraControl->transform.position = vec3(0, 0, RADIUS * 2);
    cameraControl->transform.rotate(0, -PI, 0);
    mainState->addEntity(cameraControl);

    auto camera = make_shared<Camera>();
    camera->id = (void*)0xcab;
    camera->transform.parent(&cameraControl->transform);
    mainState->addEntity(camera);

    //Light::Group<Light::Point> point;
    //Light::Point p;
    //p.position = vec3(-50, -100, -50);
    //p.color = vec3(1);
    //p.falloff = vec2(10, 1000);
    //point.addLight(p, Light::UpdateFreq::NEVER);

    Light::Group <Light::Directional> directional;
    Light::Directional d;
    d.direction = normalize(vec3(-1, -1, -0.5f));
    d.color = vec3(1);
    directional.addLight(d, Light::UpdateFreq::NEVER);

    //renderer.lights.pointLights.setGroups({ point });
    renderer.lights.directionalLights.setGroups({ directional });
    renderer.ambientColor.value = vec3(0.1f);

    cameraNav.forward = camera->forward();

    if (DEBUG) DrawDebug::getInstance().camera(camera.get());

    noiseEntity->draw();
    normalEntity->draw();
}

bool isVisibleHorizon(const vec3 view, const vec3 boundingPoint, const float radius);
void moveCamera(Entity* cameraControl, Entity* camera, float radius);
void adjustCamera(Entity* camera, float dist);

void TessellatorTest::update(double delta) {
    const auto dt = (float)delta;

    Game::update(delta);

    //quit the game
    //if (Keyboard::keyDown(Keyboard::Key::Code::Q)) Window::close(); // pretty broken in this game

    if (Keyboard::keyPressed(Keyboard::Key::Code::Equal)) wireframe = !wireframe;

    DrawDebug::getInstance().drawDebugVector(vec3(), vec3(1, 0, 0), vec3(1, 0, 0));
    DrawDebug::getInstance().drawDebugVector(vec3(), vec3(0, 1, 0), vec3(0, 1, 0));
    DrawDebug::getInstance().drawDebugVector(vec3(), vec3(0, 0, 1), vec3(0, 0, 1));

    auto cam = Camera::main;

    moveCamera(cameraControl.get(), cam, RADIUS);

    auto pos = Camera::main->transform.getComputed()->position();
    auto dist = glm::length(pos) - RADIUS;

    int activeCounter = 0;
    for (const auto& plane : planes) {
        //DrawDebug::getInstance().drawDebugVector(plane.entity->transform.getComputed()->position(), plane.boundingPoint);
        if (isVisibleHorizon(pos, plane.boundingPoint, RADIUS)) {
            if (cam->sphereInFrustum(plane.center, plane.radius)) {
                plane.entity->active = true;
                ++activeCounter;
            }
            else goto NOT_VISIBLE;
        }
        else {
        NOT_VISIBLE:
            plane.entity->active = false;
        }
    }

    // replace with higher res model swap in
    // float scaleFactor;
    // if      (dist < 0.25f) scaleFactor = 1.5;
    // else if (dist < 0.75f) scaleFactor = 3.5;
    // else                   scaleFactor = 7.5;
    
    vec3 forward;
    {
        auto t = cam->transform.getComputed();
        pos = t->position();
        forward = t->forward();
    }
    controlText->setMessage(to_string(pos, 3)
                          + "\n" + std::to_string(glm::length(pos))
                          + "\n" + to_string(quat::getEuler(cam->transform.getComputed()->rotation()))
                          + "\nPlanes Active: " + std::to_string(activeCounter));

    DrawDebug::getInstance().drawDebugVector(pos + forward, pos + forward + normalize(vec3(-1, -1, -0.5f)));
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
    planetData.radius.update(RADIUS); // in case of recompilation, this can be replaced later with a one-time set

    Game::draw();
    Text::render(&renderer.forward.objects);
}

// https://cesiumjs.org/2013/04/25/horizon-culling/
// checks whether [boundingPoint] is visible around the horizon of a sphere with r = [radius] from a given [view] position
bool isVisibleHorizon(const vec3 view, const vec3 boundingPoint, const float radius)
{
    const auto toCenter = -view, toPoint = boundingPoint - view;

    const auto pdc = glm::dot(toPoint, toCenter);
    const auto distFromHorizonSq = glm::dot(toCenter, toCenter) - radius * radius;

    // first determine if in front of horizon, then check if inside the cone
    return pdc < distFromHorizonSq || pdc * pdc / glm::dot(toPoint, toPoint) < distFromHorizonSq;
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
        const auto lateralSpeed = 2.f * RADIUS * towardSpeed;

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
