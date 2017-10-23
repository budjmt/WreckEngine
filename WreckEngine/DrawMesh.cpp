#include "DrawMesh.h"

static Render::Info gen_mat(GLtexture tex, GLprogram shader) {
    Render::Info material;
    material.setShaders(shader);
    material.setTextures(tex);
    return material;
}

DrawMesh::DrawMesh(Render::MaterialPass* r, shared<Mesh> m, const char* texFile, GLprogram shader, bool hasTangent) : DrawMesh(r, m, genTexture2D(texFile), shader, hasTangent) {}
DrawMesh::DrawMesh(Render::MaterialPass* r, shared<Mesh> m, GLtexture tex, GLprogram shader, bool hasTangent) : DrawMesh(r, m, gen_mat(tex, shader), hasTangent) {}

DrawMesh::DrawMesh(Render::MaterialPass* r, shared<Mesh> m, Render::Info _material, bool hasTangent) : _mesh(m) { 
    renderer = r; 
    material = std::move(_material);
    setupMesh(hasTangent);
    setupMaterial();
}

void DrawMesh::setupMesh(bool hasTangent) {
    auto renderData = _mesh->getRenderData(hasTangent);

    vArray.create();
    vArray.bind();

    vertBuffer.create(GL_ARRAY_BUFFER);
    vertBuffer.bind();
    vertBuffer.data(sizeof(GLfloat) * renderData->vbuffer.size(), renderData->vbuffer.data());

    elBuffer.create(GL_ELEMENT_ARRAY_BUFFER);
    elBuffer.bind();
    elBuffer.data(sizeof(GLuint) * renderData->ebuffer.size(), renderData->ebuffer.data());

    //set up an attribute for how the coordinates will be read
    GLattrarr attrSetup;
    attrSetup.add<GLfloat>(FLOATS_PER_VERT);
    attrSetup.add<GLfloat>(FLOATS_PER_UV);
    attrSetup.add<GLfloat>(FLOATS_PER_NORM);
    if (hasTangent) attrSetup.add<GLfloat>(FLOATS_PER_NORM);
    //enable attributes
    attrSetup.apply();

    vArray.unbind();
}

void DrawMesh::setupMaterial() {
    worldMatrix   = material.addResource<mat4>("worldMatrix");
    iTworldMatrix = material.addResource<mat4>("iTworldMatrix");
    _color = material.addResource<vec4>("tint");
    color(vec4(1));
}

void DrawMesh::draw(const mat4& world, Entity* entity) {
    Renderable::draw(world, entity);
    renderer->scheduleDrawElements(renderGroup, entity, &vArray, &material, tesselPrim, _mesh->getRenderData()->ebuffer.size(), GLtype<uint32_t>());
}