#include "CubemapTest.h"
#include "DrawMesh.h"
#include "HotSwap.h"
#include "TextEntity.h"
#include "ComputeEntity.h"

static constexpr float viewDistance   = 10.0f;
static constexpr float tessLevelInner = 16.0f;
static constexpr float tessLevelOuter = 16.0f;
static constexpr GLuint texSize = 256;

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
    renderData.radius.value = 5.0f;

    // Setup the compute material
    auto program = HotSwap::Shader::create();
    program->compute = Shader("Shaders/cubemap_c.glsl", GL_COMPUTE_SHADER);
    program->setupProgram();
    renderData.program = program->program();
    renderData.program.use();
    renderData.compTime = renderData.program.getUniform<float>("Time");
    renderData.compZoom = renderData.program.getUniform<float>("Zoom");
    renderData.compZoom.update(6.0f);

    // Setup the cubemap
    renderData.cubemap.create(GL_TEXTURE_CUBE_MAP);
    renderData.cubemap.bind();
    renderData.cubemap.param(GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    renderData.cubemap.param(GL_TEXTURE_MAG_FILTER, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    renderData.cubemap.set2DAs(GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_FLOAT, nullptr, texSize, texSize, GL_RGBA, GL_RGBA32F);
    renderData.cubemap.set2DAs(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, GL_FLOAT, nullptr, texSize, texSize, GL_RGBA, GL_RGBA32F);
    renderData.cubemap.set2DAs(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_FLOAT, nullptr, texSize, texSize, GL_RGBA, GL_RGBA32F);
    renderData.cubemap.set2DAs(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, GL_FLOAT, nullptr, texSize, texSize, GL_RGBA, GL_RGBA32F);
    renderData.cubemap.set2DAs(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_FLOAT, nullptr, texSize, texSize, GL_RGBA, GL_RGBA32F);
    renderData.cubemap.set2DAs(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, GL_FLOAT, nullptr, texSize, texSize, GL_RGBA, GL_RGBA32F);

    // Setup the cube mesh
    auto cubeMesh = loadOBJ("Assets/cube.obj");
    auto cubeDrawMesh = make_shared<DrawMesh>(&renderer.forward.objects, cubeMesh, renderData.cubemap, renderData.material);
    cubeDrawMesh->tesselPrim = GL_PATCHES;
    cubeDrawMesh->renderGroup = renderer.forward.objects.addGroup([] {
        GL_CHECK(glEnable(GL_CULL_FACE));
        //GLsynchro::barrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }, [] {
        GL_CHECK(glDisable(GL_CULL_FACE));
    });
    cubeDrawMesh->material.addResource(&renderData.tessLevelInner);
    cubeDrawMesh->material.addResource(&renderData.tessLevelOuter);
    cubeDrawMesh->material.addResource(&renderData.radius);
    cube = make_shared<Entity>(cubeDrawMesh);
    mainState->addEntity(cube);

    // Setup the compute entity
    auto computeEntity = make_shared<ComputeTextureEntity>();
    computeEntity->program = renderData.program;
    computeEntity->dispatchSize = {texSize, texSize, 6};
    computeEntity->update_uniforms = [&]() {
        renderData.compTime.update(time);
        renderData.compZoom.update(cosRange(time * 0.375f, 1, 8));
    };
    computeEntity->texture = renderData.cubemap;
    mainState->addEntity(computeEntity);

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
