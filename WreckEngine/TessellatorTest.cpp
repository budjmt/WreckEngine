#include "TessellatorTest.h"

#include "ColliderEntity.h"
#include "HotSwap.h"
#include "TextEntity.h"
#include "ComputeEntity.h"

bool isVisibleHorizon(const vec3 view, const vec3 boundingPoint, const float radius);

constexpr float RADIUS = 2;
bool wireframe = false;
static GLresource<float> exposure;

struct RenderData {
    GLprogram prog;
    GLuniform<mat4> mat;
    GLuniform<vec3> pos;
    GLresource<float> radius;
};
static RenderData planetData;

struct {
    GLprogram prog;
    GLresource<float> tessRadius;
    GLresource<vec2> atmosRadius, K;
    GLresource<vec3> sunPos, sunColor;
    GLuniform<vec3> camPos;
    GLuniform<mat4> camMat;
} atmosData;

struct {
    GLprogram prog;
    GLresource<vec2> atmosRadius;
    GLtexture depthLookup;
} atmosLookupData;

struct {
    GLprogram prog;
    GLresource<float> zoom;
    GLresource<int> seed;
    GLtexture cubemap;
} noiseData;

struct {
    GLprogram prog;
    GLresource<float> radius;
    GLtexture cubemap;
} normalData;

static shared<TextEntity> controlText;
static shared<Entity> cameraControl;

static shared<ComputeTextureEntity> noiseEntity, normalEntity, atmosLookupEntity;

struct LightData {
    Light::Point light;
    Transform helper;
    uint32_t index;
    Light::Group<Light::Point>* group;
};
static LightData sun;

struct {
    vec3 forward;
} cameraNav;

PlanetCSphere::TerrainPlane::TerrainPlane(Entity* e, const Mesh* mesh, const vec3 dir, const float radius) : entity(e) {
    const auto& trans = e->transform;

    auto& verts = mesh->data().verts;
    std::vector<vec3> normalizedVerts(verts.size());
    std::transform(verts.begin(), verts.end(), normalizedVerts.begin(), [&trans](const auto& v) {
        return glm::normalize(trans.getTransformed(v));
    });

    boundingSphere.center = Mesh::getCentroid(normalizedVerts) * radius;

    const auto dims = Mesh::getPreciseDims(normalizedVerts);
    boundingSphere.radius = maxf(maxf(dims.x, dims.y), dims.z) * radius * 0.5f;

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

// returns 1 if visible, 0 otherwise
int PlanetCSphere::TerrainPlane::update(const vec3& pos, const Camera* cam, const float radius) {
    if (isVisibleHorizon(pos, boundingPoint, radius)) {
        if (cam->sphereInFrustum(boundingSphere.center, boundingSphere.radius)) {
            entity->active = true;
            return 1;
        }
    }
    entity->active = false;
    return 0;
}

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

shared<Entity> PlanetCSphere::genPlane(const vec3 dir, Render::MaterialPass* renderer, const size_t group, std::function<void(DrawMesh*)>& drawSetup) {
    static auto planeMesh = loadOBJ("Assets/plane.obj");

    auto dm = make_shared<DrawMesh>(renderer, planeMesh, "Assets/texture.png", prog);
    dm->tesselPrim = GL_PATCHES;
    dm->renderGroup = group;
    drawSetup(dm.get());

    auto plane = make_shared<Entity>(dm);
    plane->active = false;
    
    plane->transform.position = dir * radius;
    plane->transform.rotation = quat(rotateBetween(vec3(0, 0, 1), dir));
    const auto scale = radius * 2.02f;
    plane->transform.scale = vec3(scale, scale, 1);

    planes.push_back(TerrainPlane(plane.get(), planeMesh.get(), dir, radius));
    return plane;
}

PlanetCSphere::PlanetCSphere(const float _radius, GLprogram _prog
                           , State* state, Render::MaterialPass* renderer, const size_t group, std::function<void(DrawMesh*)> drawSetup) : radius(_radius), prog(_prog) {
    static const vec3 cubeDirs[] = { vec3(0, 0, 1), vec3(0,  0, -1)
                                   , vec3(0, 1, 0), vec3(0, -1,  0)
                                   , vec3(1, 0, 0), vec3(-1, 0,  0) };
    for (auto& dir : cubeDirs)
        state->addEntity(genPlane(dir, renderer, group, drawSetup));
};

// returns the number of active planes
int PlanetCSphere::update(const vec3& pos, const Camera* cam) {
    if (!active) return 0;

    int activeCounter = 0;
    for (auto& plane : planes) {
        //DrawDebug::getInstance().drawDebugVector(plane.entity->transform.getComputed()->position(), plane.boundingPoint);
        activeCounter += plane.update(pos, cam, radius);
    }
    return activeCounter;
}

inline int genPlanetSeed() { return static_cast<int>(Random::get()); }

TessellatorTest::TessellatorTest() : Game(6) {
    auto mainState = make_shared<State>("main");
    addState(mainState);

    controlText = make_shared<TextEntity>("", "arial.ttf", Text::Justify::MIDDLE, Text::Justify::START, 48);
    controlText->transform.position = vec3(25, 80, 0);
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
    noiseData.seed = noiseData.prog.getUniform<int>("Seed");
    noiseData.seed.value = genPlanetSeed();

    constexpr size_t texSize = 1024; // guaranteed minimum max texture size by GL 4 is 1024
    constexpr GLenum noiseCubemapFormat = GL_RGBA32F;
    initCubemap(noiseData.cubemap, GL_FLOAT, texSize, texSize, GL_RGBA, noiseCubemapFormat);

    auto computeDispatcher = make_shared<GraphicsWorker>();
    computeDispatcher->material.setShaders(noiseData.prog, &noiseData.seed);
    computeDispatcher->material.setTextures();

    noiseEntity = make_shared<ComputeTextureEntity>(computeDispatcher);
    noiseEntity->dispatchSize = { texSize, texSize, 6 };
    noiseEntity->addImage(noiseData.cubemap, GL_WRITE_ONLY, noiseCubemapFormat);
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
    normalData.radius = normalData.prog.getUniform<float>("Radius");
    normalData.radius.value = RADIUS;

    constexpr GLenum normalCubemapFormat = GL_RGBA32F;
    initCubemap(normalData.cubemap, GL_FLOAT, texSize, texSize, GL_RGBA, normalCubemapFormat);

    computeDispatcher = make_shared<GraphicsWorker>();
    computeDispatcher->material.setShaders(normalData.prog, &normalData.radius);
    computeDispatcher->material.setTextures(noiseData.cubemap);

    normalEntity = make_shared<ComputeTextureEntity>(computeDispatcher);
    normalEntity->dispatchSize = { texSize, texSize, 6 };
    normalEntity->addImage(normalData.cubemap, GL_WRITE_ONLY, normalCubemapFormat);
    normalEntity->updateFreq = 0.f;
    //normalEntity->synchronize = false;
    //mainState->addEntity(normalEntity);
    //normalEntity->draw();

    checkProgLinkError(normalData.prog);

    atmosLookupData.depthLookup.create(GL_TEXTURE_2D);
    atmosLookupData.depthLookup.bind();
    atmosLookupData.depthLookup.param(GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T, GL_TEXTURE_WRAP_R, GL_REPEAT);
    atmosLookupData.depthLookup.param(GL_TEXTURE_MAG_FILTER, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    auto atmosLookupProg = HotSwap::Shader::create([&] {
        atmosLookupEntity->draw();
    });
    atmosLookupProg->compute = ShaderRes("Shaders/atmosScatter_c.glsl", GL_COMPUTE_SHADER);
    atmosLookupProg->setupProgram();

    atmosLookupData.prog = atmosLookupProg->program();
    atmosLookupData.prog.use();
    atmosLookupData.atmosRadius = atmosLookupData.prog.getUniform<vec2>("atmosRadius");
    atmosLookupData.atmosRadius.value = vec2(RADIUS, RADIUS + 2);

    computeDispatcher = make_shared<GraphicsWorker>();
    computeDispatcher->material.setShaders(atmosLookupData.prog, &atmosLookupData.atmosRadius);
    computeDispatcher->material.setTextures();

    constexpr int lookupSize = 512;
    atmosLookupEntity = make_shared<ComputeTextureEntity>(computeDispatcher);
    atmosLookupEntity->dispatchSize = { lookupSize, lookupSize, 1 };
    atmosLookupEntity->addImage(atmosLookupData.depthLookup, GL_WRITE_ONLY, GL_RG32F);
    atmosLookupEntity->updateFreq = 0.f;

    checkProgLinkError(atmosLookupData.prog);

    auto tessProg = HotSwap::Shader::create();
    tessProg->vertex      = ShaderRes("Shaders/planet_v.glsl",  GL_VERTEX_SHADER);
    tessProg->tessControl = ShaderRes("Shaders/planet_tc.glsl", GL_TESS_CONTROL_SHADER);
    tessProg->tessEval    = ShaderRes("Shaders/planet_te.glsl", GL_TESS_EVALUATION_SHADER);
    tessProg->fragment    = ShaderRes("Shaders/planet_f.glsl",  GL_FRAGMENT_SHADER);
    tessProg->setupProgram();

    planetData.prog = tessProg->program();
    checkProgLinkError(planetData.prog);

    planetData.prog.use();
    planetData.mat    = planetData.prog.getUniform<mat4>("cameraMatrix");
    planetData.pos    = planetData.prog.getUniform<vec3>("camPos");
    planetData.radius = planetData.prog.getUniform<float>("Radius");
    planetData.radius.value = RADIUS;

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

    surface = make_unique<PlanetCSphere>(RADIUS, planetData.prog
        , mainState.get(), rendererUsed, planetGroup, [](DrawMesh* dm) {
            dm->material.addResource(&planetData.radius);
            dm->material.addTexture(noiseData.cubemap);
            dm->material.addTexture(normalData.cubemap);
        });

    auto atmosProg = HotSwap::Shader::create();
    atmosProg->vertex      = tessProg->vertex;
    atmosProg->tessControl = tessProg->tessControl;
    atmosProg->tessEval    = ShaderRes("Shaders/atmosScatter_te.glsl", GL_TESS_EVALUATION_SHADER);
    atmosProg->fragment    = ShaderRes("Shaders/atmosScatter_f.glsl", GL_FRAGMENT_SHADER);
    atmosProg->setupProgram();

    atmosData.prog = atmosProg->program();
    checkProgLinkError(atmosData.prog);

    atmosData.prog.use();
    atmosData.camPos = atmosData.prog.getUniform<vec3>("camPos");
    atmosData.camMat = atmosData.prog.getUniform<mat4>("cameraMatrix");

    atmosData.tessRadius  = atmosData.prog.getUniform<float>("Radius");
    atmosData.K           = atmosData.prog.getUniform<vec2>("K_rm");
    atmosData.atmosRadius = atmosData.prog.getUniform<vec2>("atmosRadius");
    atmosData.sunPos      = atmosData.prog.getUniform<vec3>("sunPos");
    atmosData.sunColor    = atmosData.prog.getUniform<vec3>("sunColor");

    atmosData.K.value = vec2(1);
    atmosData.atmosRadius.value = atmosLookupData.atmosRadius.value;
    atmosData.tessRadius.value = atmosData.atmosRadius.value.y;

    
    const auto atmosGroup = renderer.forward.objects.addGroup([] {
        GL_CHECK(glDisable(GL_CULL_FACE)); // this should really save state and restore afterward
        GLsynchro::barrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }, [] {
    });
    atmosphere = make_unique<PlanetCSphere>(RADIUS + 2, atmosData.prog
        , mainState.get(), &renderer.forward.objects, atmosGroup, [](DrawMesh* dm) {
        dm->material.addResource(&atmosData.tessRadius);
        dm->material.addResource(&atmosData.atmosRadius);
        dm->material.addResource(&atmosData.K);
        dm->material.addResource(&atmosData.sunPos);
        dm->material.addResource(&atmosData.sunColor);
        dm->material.addTexture(atmosLookupData.depthLookup);
    });

    cameraControl = make_shared<TransformEntity>();
    cameraControl->transform.position = vec3(0, 0, RADIUS * 2);
    cameraControl->transform.rotate(0, -PI, 0);
    mainState->addEntity(cameraControl);

    auto camera = make_shared<Camera>();
    camera->id = (void*)0xcab;
    camera->zfar = 2000.f;
    camera->transform.parent(&cameraControl->transform);
    mainState->addEntity(camera);

    Light::Group<Light::Point> point;
    sun.light.position = sun.helper.position = vec3(-50, -100, -50);
    sun.helper.rotation *= quat(rotateBetween(vec3(0, 0, 1), -glm::normalize(sun.helper.position())));
    sun.light.color = vec3(1);
    sun.light.falloff = vec2(100, 500);
    sun.light.tag = 1;
    point.addLight(sun.light, Light::UpdateFreq::SOMETIMES);

    // shut the performance warning from the point/spot only bug up for now
    Light::Group<Light::Spotlight> spot;
    Light::Spotlight s;
    s.isOff = true;
    spot.addLight(s, Light::UpdateFreq::NEVER);
    renderer.lights.spotLights.setGroups({ spot });

    //Light::Group <Light::Directional> directional;
    //Light::Directional d;
    //d.direction = normalize(vec3(-1, -1, -0.5f));
    //d.color = vec3(1);
    //directional.addLight(d, Light::UpdateFreq::NEVER);

    renderer.lights.pointLights.setGroups({ point });
    //renderer.lights.directionalLights.setGroups({ directional });
    renderer.ambientColor.value = vec3(0.1f);

    sun.index = point.getLightIndexByTag(1, Light::UpdateFreq::SOMETIMES);
    sun.group = &renderer.lights.pointLights.getGroup(0);

    cameraNav.forward = camera->forward();

    if (DEBUG) DrawDebug::getInstance().camera(camera.get());

    noiseEntity->draw();
    normalEntity->draw();
    atmosLookupEntity->draw();

    setupPostProcess();
    exposure.value = 2;
}

void TessellatorTest::setupPostProcess() {
    using namespace Render;

    renderer.forward.postProcess.output = Target::create<GLubyte>();

    auto& colorRender = gBuffer[0];

    // HDR
    auto hdr = make_shared<PostProcess>();
    auto hdrProg = PostProcess::make_program("Shaders/postProcess/hdr.glsl");
    exposure = hdrProg.getUniform<float>("exposure");
    hdr->data.setShaders(hdrProg, &exposure);
    hdr->data.setTextures(colorRender);
    hdr->renderToTextures(renderer.forward.postProcess.output);

    renderer.forward.postProcess.entry.chainsTo(hdr);
}

void moveCamera(Entity* cameraControl, Entity* camera, float radius);
void adjustCamera(Entity* camera, float dist);
void moveSun(float radius);

void TessellatorTest::update(double delta) {
    const auto dt = (float)delta;

    Game::update(delta);

    //quit the game
    //if (Keyboard::keyDown(Keyboard::Key::Code::Q)) Window::close(); // pretty broken in this game

    if (Keyboard::keyPressed(Keyboard::Key::Code::Equal)) wireframe = !wireframe;
    if (Keyboard::keyPressed(Keyboard::Key::Code::Space)) {
        noiseData.seed.value = genPlanetSeed();
        Thread::Render::runNextFrame([]() {
            noiseEntity->draw();
            normalEntity->draw();
        });
    }

    DrawDebug::getInstance().drawDebugVector(vec3(), vec3(1, 0, 0), vec3(1, 0, 0));
    DrawDebug::getInstance().drawDebugVector(vec3(), vec3(0, 1, 0), vec3(0, 1, 0));
    DrawDebug::getInstance().drawDebugVector(vec3(), vec3(0, 0, 1), vec3(0, 0, 1));

    auto cam = Camera::main;

    moveCamera(cameraControl.get(), cam, RADIUS);
    moveSun(RADIUS);
    atmosData.sunPos.value = sun.light.position;
    atmosData.sunColor.value = sun.light.color;

    auto pos = Camera::main->transform.getComputed()->position();
    auto dist = glm::length(pos) - RADIUS;

    int activeCounter = surface->update(pos, cam);
    atmosphere->update(pos, cam);

    // replace with higher res model swap in
    // float scaleFactor;
    // if      (dist < 0.25f) scaleFactor = 1.5;
    // else if (dist < 0.75f) scaleFactor = 3.5;
    // else                   scaleFactor = 7.5;
    
    if (Keyboard::keyDown(Keyboard::Key::Code::RBracket)) exposure.value += 10 * dt;
    if (Keyboard::keyDown(Keyboard::Key::Code::LBracket)) exposure.value -= 10 * dt;

    if (Keyboard::keyPressed(Keyboard::Key::Code::F1)) atmosphere->setActive(!atmosphere->active);

    vec3 forward;
    {
        auto t = cam->transform.getComputed();
        pos = t->position();
        forward = t->forward();
    }
    controlText->setMessage(to_string(pos, 3)
                          + "\n" + std::to_string(glm::length(pos))
                          + "\n" + to_string(quat::getEuler(cam->transform.getComputed()->rotation()))
                          + "\nPlanes Active: " + std::to_string(activeCounter)
                          + "\nSeed: " + std::to_string(noiseData.seed.value)
                          + "\nExposure: " + std::to_string(exposure.value));

    //DrawDebug::getInstance().drawDebugVector(pos + forward, pos + forward + normalize(vec3(-1, -1, -0.5f)));
}

void TessellatorTest::postUpdate() {
    Game::postUpdate();
    Text::postUpdate();
}

void TessellatorTest::draw() {
    auto mat = Camera::main->getCamMat();
    auto pos = Camera::main->transform.getComputed()->position;
    planetData.prog.use();
    planetData.mat.update(mat);
    planetData.pos.update(pos);
    atmosData.prog.use();
    atmosData.camMat.update(mat);
    atmosData.camPos.update(pos);

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

bool controlAroundPlanet(Transform& transform, const float centerDist, const float towardSpeed, const float dt,
    Keyboard::Key::Code upKey, Keyboard::Key::Code downKey, Keyboard::Key::Code leftKey, Keyboard::Key::Code rightKey) {
    // move around the surface
    const auto lateralSpeed = 2.f * RADIUS * towardSpeed;

    float dH = 0, dV = 0;
    bool moved = false;

    if      (Keyboard::keyDown(upKey))    { dV =  lateralSpeed * dt; moved = true; }
    else if (Keyboard::keyDown(downKey))  { dV = -lateralSpeed * dt; moved = true; }
    if      (Keyboard::keyDown(leftKey))  { dH = -lateralSpeed * dt; moved = true; }
    else if (Keyboard::keyDown(rightKey)) { dH =  lateralSpeed * dt; moved = true; }

    const auto getRot = [](float d, float radius) {
        return 2 * asin(d * 0.5f / radius);
    };

    if (moved) {
        transform.rotate(getRot(dV, centerDist), getRot(dH, centerDist), 0);
        transform.position = transform.forward() * -centerDist;
    }
    return moved;
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
        bool camMoved = false;

        if      (Keyboard::keyDown(Keyboard::Key::W)) { cameraControl->transform.position -= pos * (towardSpeed * dt); camMoved = true; }
        else if (Keyboard::keyDown(Keyboard::Key::S)) { cameraControl->transform.position += pos * (towardSpeed * dt); camMoved = true; }

        if (camMoved) {
            pos = camera->transform.getComputed()->position();
            auto dist = glm::length(pos) - radius;
            adjustCamera(camera, dist);
        }
    }
    else {
        controlAroundPlanet(cameraControl->transform, centerDist, towardSpeed, dt
                          , Keyboard::Key::Code::W, Keyboard::Key::Code::S, Keyboard::Key::Code::A, Keyboard::Key::Code::D);
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

void moveSun(float radius) {
    auto dt = (float)Time::delta;

    auto pos = sun.helper.position();
    auto centerDist = glm::length(pos);

    const auto towardSpeed = (centerDist - radius) / centerDist;

    const bool shift = Keyboard::shiftDown();
    bool moved = false;

    if (shift) {
        // move away from/towards the surface
        moved = true;
        if      (Keyboard::keyDown(Keyboard::Key::I)) sun.light.position = sun.helper.position -= pos * (towardSpeed * dt);
        else if (Keyboard::keyDown(Keyboard::Key::K)) sun.light.position = sun.helper.position += pos * (towardSpeed * dt);
        else moved = false;
    }
    else {
        moved = controlAroundPlanet(sun.helper, centerDist, towardSpeed * 50, dt
            , Keyboard::Key::Code::I, Keyboard::Key::Code::K, Keyboard::Key::Code::J, Keyboard::Key::Code::L);
        if (moved)
            sun.light.position = sun.helper.position();
    }

    if (moved) Thread::Render::runNextFrame([] { sun.group->updateLight(sun.index, Light::UpdateFreq::SOMETIMES, sun.light); });
    DrawDebug::getInstance().drawDebugSphere(sun.helper.position, sun.light.falloff.x, vec3(1));
    DrawDebug::getInstance().drawDebugSphere(sun.helper.position, sun.light.falloff.y, vec3(1,1,0));
}