#include "CubemapTest.h"
#include "DrawMesh.h"
#include "HotSwap.h"
#include "TextEntity.h"
#include "ComputeEntity.h"

static constexpr float viewDistance   = 10.0f;
static constexpr float tessLevelInner = 16.0f;
static constexpr float tessLevelOuter = 16.0f;
static constexpr GLuint texSize = 256;
static constexpr float radius = 5.0f;

inline float cosRange(float val, float min, float max)
{
    return (cosf(val) * 0.5f + 0.5f) * (max - min) + min;
}

CubemapTest::CubemapTest() : Game(6)
{
    auto mainState = make_shared<State>("main");
    addState(mainState);

    GLframebuffer::setClearColor(1, 0, 0, 1);

    // Setup the camera
    camera = make_shared<Camera>();
    camera->id = reinterpret_cast<void*>(0xbeefc0de);
    camera->transform.position = vec3(0, 0, viewDistance);
    camera->transform.rotate(0, PI, 0);
    mainState->addEntity(camera);

    // Setup the render material
    auto material = HotSwap::Shader::create();
    using Shader = decltype(material->vertex);
    material->vertex      = Shader("Shaders/cubemap_v.glsl",  GL_VERTEX_SHADER);
    material->tessControl = Shader("Shaders/cubemap_tc.glsl", GL_TESS_CONTROL_SHADER);
    material->tessEval    = Shader("Shaders/cubemap_te.glsl", GL_TESS_EVALUATION_SHADER);
    material->fragment    = Shader("Shaders/cubemap_f.glsl",  GL_FRAGMENT_SHADER);
    material->setupProgram();

    renderData.material = material->program();
    renderData.viewProjection = renderData.material.getUniform<mat4>("ViewProjection");
    renderData.tessLevelInner = GLresource<float>(renderData.material, "TessLevelInner");
    renderData.tessLevelOuter = GLresource<float>(renderData.material, "TessLevelOuter");
    renderData.radius         = GLresource<float>(renderData.material, "Radius");

    // Set initial uniform values
    renderData.tessLevelInner.value = tessLevelInner;
    renderData.tessLevelOuter.value = tessLevelOuter;
    renderData.radius.value = radius;

    // Setup the noise compute material
    auto program = HotSwap::Shader::create();
    program->compute = Shader("Shaders/noisemap_c.glsl", GL_COMPUTE_SHADER);
    program->setupProgram();
    noiseMap.prog = program->program();
    noiseMap.prog.use();
	noiseMap.zoom = GLresource<float, true>(noiseMap.prog, "Zoom", [&] {
		return cosRange(time * 0.375f, 1, 8);
	});

    // Setup the normal map compute material
    program = HotSwap::Shader::create();
    program->compute = Shader("Shaders/normalmap_c.glsl", GL_COMPUTE_SHADER);
    program->setupProgram();
    normalMap.prog = program->program();
    normalMap.prog.use();
	normalMap.camPos = GLresource<vec3, true>(normalMap.prog, "CameraPosition", [&] {
		return Camera::main->transform.position();
	});
    normalMap.radius = normalMap.prog.getUniform<float>("Radius");
    normalMap.radius.value = radius;

    // TODO - Michael's going to want to make that genCubeMap function

    // Setup the noise cubemap
    noiseMap.tex.create(GL_TEXTURE_CUBE_MAP);
    noiseMap.tex.bind();
    noiseMap.tex.param(GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    noiseMap.tex.param(GL_TEXTURE_MAG_FILTER, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    noiseMap.tex.set2DAs(GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_FLOAT, nullptr, texSize, texSize, GL_RGBA, GL_RGBA32F);
    noiseMap.tex.set2DAs(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, GL_FLOAT, nullptr, texSize, texSize, GL_RGBA, GL_RGBA32F);
    noiseMap.tex.set2DAs(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_FLOAT, nullptr, texSize, texSize, GL_RGBA, GL_RGBA32F);
    noiseMap.tex.set2DAs(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, GL_FLOAT, nullptr, texSize, texSize, GL_RGBA, GL_RGBA32F);
    noiseMap.tex.set2DAs(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_FLOAT, nullptr, texSize, texSize, GL_RGBA, GL_RGBA32F);
    noiseMap.tex.set2DAs(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, GL_FLOAT, nullptr, texSize, texSize, GL_RGBA, GL_RGBA32F);

    // Setup the normal cubemap
    normalMap.tex.create(GL_TEXTURE_CUBE_MAP);
    normalMap.tex.bind();
    normalMap.tex.param(GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    normalMap.tex.param(GL_TEXTURE_MAG_FILTER, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    normalMap.tex.set2DAs(GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_FLOAT, nullptr, texSize, texSize, GL_RGBA, GL_RGBA32F);
    normalMap.tex.set2DAs(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, GL_FLOAT, nullptr, texSize, texSize, GL_RGBA, GL_RGBA32F);
    normalMap.tex.set2DAs(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_FLOAT, nullptr, texSize, texSize, GL_RGBA, GL_RGBA32F);
    normalMap.tex.set2DAs(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, GL_FLOAT, nullptr, texSize, texSize, GL_RGBA, GL_RGBA32F);
    normalMap.tex.set2DAs(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_FLOAT, nullptr, texSize, texSize, GL_RGBA, GL_RGBA32F);
    normalMap.tex.set2DAs(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, GL_FLOAT, nullptr, texSize, texSize, GL_RGBA, GL_RGBA32F);

    // Setup the noise map compute entity
	auto computeDispatcher = make_shared<GraphicsWorker>();
	computeDispatcher->material.setShaders(noiseMap.prog, &noiseMap.zoom);
	computeDispatcher->material.setTextures();

    auto noiseEntity = make_shared<ComputeTextureEntity>(computeDispatcher);
    noiseEntity->dispatchSize = { texSize, texSize, 6 };
    noiseEntity->texture = noiseMap.tex;
    noiseEntity->index = 0;
    mainState->addEntity(noiseEntity);

    // Setup the normal map compute entity
	computeDispatcher = make_shared<GraphicsWorker>();
	computeDispatcher->material.setShaders(normalMap.prog, &normalMap.camPos);
	computeDispatcher->material.setTextures();

    auto normalEntity = make_shared<ComputeTextureEntity>(computeDispatcher);
    normalEntity->dispatchSize = { texSize, texSize, 6 };
    normalEntity->texture = normalMap.tex;
    normalEntity->index = 1;
    normalEntity->updateFreq = 0.0f;
    mainState->addEntity(normalEntity);

    // Setup the cube mesh
    auto cubeMesh = loadOBJ("Assets/cube.obj");
    auto cubeDrawMesh = make_shared<DrawMesh>(&renderer.forward.objects, cubeMesh, noiseMap.tex, renderData.material);
    cubeDrawMesh->tesselPrim = GL_PATCHES;
    cubeDrawMesh->renderGroup = renderer.forward.objects.addGroup([] {
        GL_CHECK(glEnable(GL_CULL_FACE));
        GLsynchro::barrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }, [] {
        GL_CHECK(glDisable(GL_CULL_FACE));
    });
    cubeDrawMesh->material.addTexture(normalMap.tex);
    cubeDrawMesh->material.addResource(&renderData.tessLevelInner);
    cubeDrawMesh->material.addResource(&renderData.tessLevelOuter);
    cubeDrawMesh->material.addResource(&renderData.radius);
    cube = make_shared<Entity>(cubeDrawMesh);
    mainState->addEntity(cube);

    renderer.lightingOn = false;

    if (DEBUG) DrawDebug::getInstance().camera(camera.get());
}

void CubemapTest::update(double dt)
{
    time += static_cast<float>(dt);
    vec3 axis = {0, 1, 0};
    axis = glm::normalize(axis);
    cube->transform.rotate((float)dt, axis);

    if (Keyboard::keyPressed(Keyboard::Key::Escape))
    {
        Window::close();
    }
    else if (Keyboard::keyPressed(Keyboard::Key::Space))
    {
        rotate = !rotate;
    }
    else if (Keyboard::keyPressed(Keyboard::Key::Backspace))
    {
        direction = -direction;
    }

    Game::update(dt);
}

void CubemapTest::postUpdate()
{
    Game::postUpdate();
    // Here in case text is needed
}

void CubemapTest::draw()
{
    // TODO - Only set camera matrix if it's been updated
    renderData.material.use();
    renderData.viewProjection.update(camera->getCamMat());

    Game::draw();
}
