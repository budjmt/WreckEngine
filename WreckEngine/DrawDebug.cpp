#include "DrawDebug.h"
#include <iostream>

DrawDebug::DrawDebug() {
#if DEBUG
    auto vecShader  = loadProgram("Shaders/_debug/vecvertexShader.glsl", "Shaders/_debug/vecfragmentShader.glsl");
    auto meshShader = loadProgram("Shaders/_debug/meshvertexShader.glsl", "Shaders/_debug/meshfragmentShader.glsl");
    
    arrow = loadOBJ("Assets/_debug/arrow.obj");
    if (!arrow) {
        genCone("Assets/_debug/arrow.obj", 8);
        arrow = loadOBJ("Assets/_debug/arrow.obj");
    }
    
    sphere = loadOBJ("Assets/_debug/sphere.obj");
    if (!sphere) {
        genSphere("Assets/_debug/sphere.obj", 8);
        sphere = loadOBJ("Assets/_debug/sphere.obj");
    }

    box = loadOBJ("Assets/_debug/cube.obj");
    if (!box) {
        genCube("Assets/_debug/cube.obj");
        box = loadOBJ("Assets/_debug/cube.obj");
    }

    GLattrarr attrSetup;
    const auto vecSize = sizeof(GLfloat) * FLOATS_PER_VERT;

    vecVAO.create();
    vecVAO.bind();
    vecBuffer.create(GL_ARRAY_BUFFER, GL_STREAM_DRAW);

    vecBuffer.bind();
    vecBuffer.data(vecSize * MAX_VECTORS * 4, nullptr);
    vectorInsts.reserve(MAX_VECTORS * 4);

    attrSetup.add<vec3>(2);
    attrSetup.apply();

    const auto mSetup = [](GLattrarr& attrSetup) {
        attrSetup.add<vec4>(1, 1);
        attrSetup.add<mat4>(1, 1);
    };

    arrows  = InstMesh<MeshData>(*arrow,  MAX_VECTORS, 1, mSetup);
    spheres = InstMesh<MeshData>(*sphere, MAX_SPHERES, 1, mSetup);
    boxes   = InstMesh<MeshData>(*box,    MAX_BOXES,   1, mSetup);

    vecVAO.unbind();

    vecMat.setShaders(vecShader);
    vecMat.setTextures();
    vecMat.addResource<GLcamera::matrix>("cameraMatrix");

    meshMat.setShaders(meshShader);
    meshMat.setTextures();
    meshMat.addResource<GLcamera::matrix>("cameraMatrix");
#endif
}

DrawDebug& DrawDebug::get() {
    static DrawDebug instance;
    return instance;
}

void DrawDebug::setRenderers(Render::MaterialPass* deferred, Render::MaterialPass* forward) {
    struct X { 
        X(DrawDebug* d, Render::MaterialPass* r) { 
            d->fillIndex = r->addGroup([] {
                GLstate<GL_CULL_FACE, GL_ENABLE_BIT> cull{ true };
                cull.save(); cull.apply();
            }, [] {
                GLstate<GL_CULL_FACE, GL_ENABLE_BIT>::restore();
            });
            d->wireframeIndex = r->addGroup([]() { 
                GLstate<GL_POLYGON_MODE>{ GL_LINE }.apply();
                GLstate<GL_CULL_FACE, GL_ENABLE_BIT> cull{ true };
                cull.save(); cull.apply();
            }, []() { 
                GLstate<GL_POLYGON_MODE>{ GL_FILL }.apply();
                GLstate<GL_CULL_FACE, GL_ENABLE_BIT>::restore();
            });
        }
    };

    static X addWireframe(this, forward);
    this->deferred = deferred;
    this->forward = forward;
}

void DrawDebug::flush() {
    debugVectors.flush();
    debugSpheres.flush();
    debugBoxes.flush();
}

void DrawDebug::postUpdate() {
    debugVectors.seal();
    debugSpheres.seal();
    debugBoxes.seal();
}

void DrawDebug::draw(Render::MaterialPass* o, Render::MaterialPass* a) {
#if DEBUG
    setRenderers(o, a);
    vecMat.apply();
    meshMat.apply();

    drawVectors();
    drawSpheres();
    drawBoxes();
#endif
}

void DrawDebug::drawVectors() {
    debugVectors.consumeAll([this](auto& debugVectors) {
        vectorInsts.insert(end(vectorInsts), begin(debugVectors), end(debugVectors));
        for (const auto& vec : debugVectors) {
            constexpr auto scaleFactor = 0.05f;

            const auto bet = vec.end.pos - vec.start.pos;
            const auto arrowLoc = vec.end.pos - bet * (scaleFactor * 0.5f / length(bet));

            const auto translate = glm::translate(arrowLoc);
            const auto rotate    = rotateBetween(vec3(0, 1, 0), normalize(bet));
            const auto scale     = glm::scale(vec3(0.5f, 1.f, 0.5f) * scaleFactor);
            arrows.instances.push_back({ vec4(vec.end.color, 1), translate * rotate * scale });
        }
    });

    vecBuffer.bind();
    vecBuffer.data(vectorInsts.data());
    forward->scheduleDrawArrays(wireframeIndex, nullptr, &vecVAO, &vecMat, GL_LINES, vectorInsts.size() * 2); // there are 2 points per vector fed as elements to GL_LINES

    vectorInsts.clear();
    vecsAdded -= arrows.instances.size(); // the number of complete vectors == the number of arrows rendered

    arrows.update();
    arrows.draw(forward, &meshMat, fillIndex);
    arrows.instances.clear();
}

void DrawDebug::drawSpheres() {
    debugSpheres.consumeAll([this](auto& debugSpheres) {
        for (const auto& sphere : debugSpheres) {
            const auto translate = glm::translate(sphere.center);
            const auto scale     = glm::scale(vec3(sphere.rad * 2));
            spheres.instances.push_back({ sphere.color, translate * scale });
        }
    });

    spheres.update();
    spheres.draw(forward, &meshMat, fillIndex);
    
    auto spheresRendered = spheres.instances.size();
    spheres.instances.clear();
    spheresAdded -= spheresRendered;
}

void DrawDebug::drawBoxes() {
    debugBoxes.consumeAll([this](auto& debugBoxes) {
        for (const auto& box : debugBoxes) {
            const auto translate = glm::translate(box.pos);
            const auto scale     = glm::scale(box.dims);
            boxes.instances.push_back({ box.color, translate * scale });
        }
    });

    boxes.update();
    boxes.draw(forward, &meshMat, wireframeIndex);

    auto boxesRendered = boxes.instances.size();
    boxes.instances.clear();
    boxesAdded -= boxesRendered;
}

void DrawDebug::drawDebugVector(vec3 start, vec3 end, vec3 color) {
#if DEBUG
    if (++vecsAdded > MAX_VECTORS) { --vecsAdded; return; }
    auto& v = debugVectors.get();
    v.push_back({ { start, color }, { end, color } });
#endif
}

void DrawDebug::drawDebugSphere(vec3 pos, float rad, vec3 color, float opacity) {
#if DEBUG
    if (++spheresAdded > MAX_SPHERES) { --spheresAdded; return; }
    auto& s = debugSpheres.get();
    s.push_back({ { color, opacity }, pos, rad });
#endif
}

void DrawDebug::drawDebugBox(vec3 pos, float w, float h, float d, vec3 color, float opacity) {
#if DEBUG
    if (++boxesAdded > MAX_BOXES) { --boxesAdded; return; }
    auto& b = debugBoxes.get();
    b.push_back({ pos, { w, h, d }, { color, opacity } });
#endif
}