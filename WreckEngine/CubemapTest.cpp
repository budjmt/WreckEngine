#include "CubemapTest.h"
#include "DrawMesh.h"
#include "HotSwap.h"
#include "TextEntity.h"

static const mat4 identity = mat4(1.0f);
static const mat3 normalMatrix = mat3(1.0f);
static constexpr float viewDistance = 10.0f;
static constexpr float tessLevelInner = 16.0f;
static constexpr float tessLevelOuter = 16.0f;
static constexpr GLuint texSize = 256;

CubemapTest::CubemapTest()
    : Game(6)
{
    drawDebug = false; // Causes crashes when enabled

    auto mainState = make_shared<State>("main");
    addState(mainState);

    // Setup the camera
    camera = make_shared<Camera>();
    camera->id = reinterpret_cast<void*>(0xbeefc0de); // why
    calcCameraPosition();

    // Setup the render material
    auto material = HotSwap::Shader::create();
    using Shader = decltype(material->vertex);
    material->vertex      = Shader("Shaders/cubemap_v.glsl",  GL_VERTEX_SHADER);
    material->tessControl = Shader("Shaders/cubemap_tc.glsl", GL_TESS_CONTROL_SHADER);
    material->tessEval    = Shader("Shaders/cubemap_te.glsl", GL_TESS_EVALUATION_SHADER);
    material->fragment    = Shader("Shaders/cubemap_f.glsl",  GL_FRAGMENT_SHADER);
    material->setupProgram();
    renderData.material = material->getProgram();
    renderData.viewProjection = renderData.material.getUniform<mat4>("ViewProjection");
    renderData.model = renderData.material.getUniform<mat4>("Model");
    renderData.normalMatrix = renderData.material.getUniform<mat3>("NormalMatrix");
    renderData.tessLevelInner = renderData.material.getUniform<float>("TessLevelInner");
    renderData.tessLevelOuter = renderData.material.getUniform<float>("TessLevelOuter");
    renderData.radius = renderData.material.getUniform<float>("Radius");
    renderData.material.setOnce<GLsampler>("Tex", 0);

    // Set initial uniform values
    renderData.material.use();
    renderData.viewProjection.value = camera->getCamMat();
    renderData.model.value = identity;
    renderData.normalMatrix.value = normalMatrix;
    renderData.tessLevelInner.value = tessLevelInner;
    renderData.tessLevelOuter.value = tessLevelOuter;
    renderData.radius.value = 3.0f;

    // Setup the compute material
    auto program = HotSwap::Shader::create();
    program->compute = Shader("Shaders/cubemap_c.glsl", GL_COMPUTE_SHADER);
    program->setupProgram();
    renderData.program = program->getProgram();
    renderData.program.isCompute = true;

    // Setup the cubemap
    auto& cubemap = renderData.cubemap;
    cubemap.create(GL_TEXTURE_CUBE_MAP, 0);
    cubemap.bind();
    cubemap.param(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    cubemap.param(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    cubemap.param(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    cubemap.param(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    cubemap.param(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // Need to see why TF this isn't working with "cubemap.set2DAs"
    GL_CHECK(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA32F, texSize, texSize, 0, GL_RGBA, GL_FLOAT, nullptr));
    GL_CHECK(glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA32F, texSize, texSize, 0, GL_RGBA, GL_FLOAT, nullptr));
    GL_CHECK(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA32F, texSize, texSize, 0, GL_RGBA, GL_FLOAT, nullptr));
    GL_CHECK(glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA32F, texSize, texSize, 0, GL_RGBA, GL_FLOAT, nullptr));
    GL_CHECK(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA32F, texSize, texSize, 0, GL_RGBA, GL_FLOAT, nullptr));
    GL_CHECK(glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA32F, texSize, texSize, 0, GL_RGBA, GL_FLOAT, nullptr));

    // Setup the cube mesh
    auto cube = loadOBJ("Assets/cube.obj");
    auto mesh = make_shared<DrawMesh>(&renderer.forward.objects, cube, cubemap, renderData.material);
    mesh->tesselPrim = GL_PATCHES;
#if 0
    mesh->renderGroup = renderer.forward.objects.addGroup([] {
        GL_CHECK(glEnable(GL_CULL_FACE));
    }, [] {
        GL_CHECK(glDisable(GL_CULL_FACE));
    });
#endif
    mainState->addEntity(make_shared<Entity>(mesh));

    renderer.lightingOn = false;
}

void CubemapTest::calcCameraPosition()
{
    //vec3 position = {sinf(time), 0, cosf(time)};
    const vec3 position = {0, 0, 1};
    camera->transform.position = glm::normalize(position) * viewDistance;
    camera->transform.makeDirty();
}

void CubemapTest::update(double dt)
{
    if (rotate)
    {
        time += static_cast<float>(dt) * direction;
        calcCameraPosition();
    }

    if (Keyboard::keyDown(Keyboard::Key::Escape))
    {
        Window::close();
    }
}

void CubemapTest::postUpdate()
{
    Game::postUpdate();
    // Here in case text is needed
}

void CubemapTest::draw()
{
    // Probably won't need to do this every frame in production, but ¯\_(ツ)_/¯
    // TODO - Talk to Michael about being able to have this in update
    renderData.cubemap.bindImage(GL_WRITE_ONLY, GL_RGBA32F, 0, 0, GL_TRUE, 0);
    renderData.program.use();
    renderData.program.dispatch(texSize, texSize, 6);
    GLsynchro::barrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    // TODO - Only set camera matrix if it's been updated
    renderData.material.use();
    renderData.viewProjection.value = camera->getCamMat();

    Game::draw();
}
