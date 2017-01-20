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

    arrows = InstMesh<m_MeshData>(arrow.get(), MAX_VECTORS, 1, mSetup);
    spheres = InstMesh<m_MeshData>(sphere.get(), MAX_SPHERES, 1, mSetup);
    boxes = InstMesh<m_MeshData>(box.get(), MAX_BOXES, 1, mSetup);

    vecVAO.unbind();

    vecCam  = vecShader.getUniform<mat4>("cameraMatrix");
    meshCam = meshShader.getUniform<mat4>("cameraMatrix");

    vecMat.setShaders(vecShader);
    vecMat.setTextures();

    meshMat.setShaders(meshShader);
    meshMat.setTextures();
#endif
}

DrawDebug& DrawDebug::getInstance() {
    static DrawDebug instance;
    return instance;
}

void DrawDebug::camera(Camera* c) { cam = c; }

void DrawDebug::setRenderers(Render::MaterialPass* deferred, Render::MaterialPass* forward) {
    struct X { 
        X(DrawDebug* d, Render::MaterialPass* r) { 
            d->wireframeIndex = r->addGroup([]() { 
                GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE)); 
                GL_CHECK(glEnable(GL_CULL_FACE));
            }, []() { 
                GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL)); 
                GL_CHECK(glDisable(GL_CULL_FACE));
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
    
    if (cam) {
        auto c = cam->getCamMat();
        vecMat.shaders->program.use();
        vecCam.update(c);
        meshMat.shaders->program.use();
        meshCam.update(c);
    }

    drawVectors();
    drawSpheres();
    drawBoxes();
#endif
}

void DrawDebug::drawVectors() {
    debugVectors.consumeAll([this](auto& debugVectors) {
        vectorInsts.insert(vectorInsts.end(), debugVectors.begin(), debugVectors.end());
        for (size_t i = 0, numVecs = debugVectors.size(); i < numVecs; i += 4) {
            constexpr auto sfact = 0.05f;
            const auto s = debugVectors[i], c1 = debugVectors[i + 1]
                , e = debugVectors[i + 2], c2 = debugVectors[i + 3];

            const auto es = e - s;
            const auto v = e - es * (sfact * 0.5f / length(es));

            const auto translate = glm::translate(v);
            const auto rotate = rotateBetween(vec3(0, 1, 0), glm::normalize(es));
            const auto scale = glm::scale(vec3(0.5f, 1.f, 0.5f) * sfact);
            arrows.instances.push_back({ vec4(c2, 1), translate * rotate * scale });
        }
    });

    vecVAO.bind();
    vecBuffer.bind();
    vecBuffer.data(vectorInsts.data());
    forward->scheduleDrawArrays(wireframeIndex, &vecVAO, &vecMat, GL_LINES, vectorInsts.size());
    
    vectorInsts.clear();
    vecsAdded -= arrows.instances.size(); // the number of complete vectors == the number of arrows rendered (vectorInsts.size() / 2)

    arrows.update();
    arrows.draw(forward, &meshMat, 0);
    arrows.instances.clear();
}

void DrawDebug::drawSpheres() {	
    debugSpheres.consumeAll([this](auto& debugSpheres) {
        for (const auto& s : debugSpheres) {
            const auto translate = glm::translate(s.center);
            const auto scale = glm::scale(vec3(s.rad * 2));
            spheres.instances.push_back({ s.color, translate * scale });
        }
    });

    spheres.update();
    spheres.draw(forward, &meshMat, 0);
    
    auto spheresRendered = spheres.instances.size();
    spheres.instances.clear();
    spheresAdded -= spheresRendered;
}

void DrawDebug::drawBoxes() {
    debugBoxes.consumeAll([this](auto& debugBoxes) {
        for (size_t i = 0, numBoxes = debugBoxes.size(); i < numBoxes; i += 3) {
            const auto translate = glm::translate(vec3(debugBoxes[i]));
            const auto scale = glm::scale(vec3(debugBoxes[i + 1]));
            boxes.instances.push_back({ debugBoxes[i + 2], translate * scale });
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
    if (vecsAdded > MAX_VECTORS) return;
    auto& v = debugVectors.get();
    v.push_back(start);
    v.push_back(color);
    v.push_back(end);
    v.push_back(color);
    ++vecsAdded;
#endif
}

void DrawDebug::drawDebugSphere(vec3 pos, float rad, vec3 color, float opacity) {
#if DEBUG
    if (spheresAdded > MAX_SPHERES) return;
    auto& s = debugSpheres.get();
    s.push_back({ vec4(color, opacity), pos, rad });
    ++spheresAdded;
#endif
}

void DrawDebug::drawDebugBox(vec3 pos, float w, float h, float d, vec3 color, float opacity) {
#if DEBUG
    if (boxesAdded > MAX_BOXES) return;
    auto& b = debugBoxes.get();
    b.push_back(vec4(pos, 0));
    b.push_back(vec4(w, h, d, 0));
    b.push_back(vec4(color, opacity));
    ++boxesAdded;
#endif
}