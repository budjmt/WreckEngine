#include "DrawMesh.h"

DrawMesh::DrawMesh(Render::MaterialPass* r, shared<Mesh> m, const char* texFile, GLprogram shader) : DrawMesh(r, m, genTexture2D(texFile), shader) {}
DrawMesh::DrawMesh(Render::MaterialPass* r, shared<Mesh> m, GLtexture tex, GLprogram shader) : _mesh(m) { renderer = r; setup(tex, shader); }
DrawMesh::DrawMesh(Render::MaterialPass* r, Mesh::FaceData& fd, Mesh::FaceIndex& fi, const char* texFile, GLprogram shader) : DrawMesh(r, make_shared<Mesh>(fd, fi), texFile, shader) {}
DrawMesh::DrawMesh(Render::MaterialPass* r, Mesh::FaceData& fd, Mesh::FaceIndex& fi, GLtexture tex, GLprogram shader) : DrawMesh(r, make_shared<Mesh>(fd, fi), tex, shader) {}

void DrawMesh::setup(GLtexture tex, GLprogram shader) {
    
    auto renderData = _mesh->getRenderData();

    vArray.create();
    vArray.bind();

    vertBuffer.create(GL_ARRAY_BUFFER);
    vertBuffer.bind();
    vertBuffer.data(sizeof(GLfloat) * renderData->vbuffer.size(), &renderData->vbuffer[0]);

    elBuffer.create(GL_ELEMENT_ARRAY_BUFFER);
    elBuffer.bind();
    elBuffer.data(sizeof(GLuint) * renderData->ebuffer.size(), &renderData->ebuffer[0]);

    //set up an attribute for how the coordinates will be read
    GLattrarr attrSetup;
    attrSetup.add<GLfloat>(FLOATS_PER_VERT);
    attrSetup.add<GLfloat>(FLOATS_PER_UV);
    attrSetup.add<GLfloat>(FLOATS_PER_NORM);
    //enable attributes
    attrSetup.apply();

    vArray.unbind();

    shader.use();
    worldMatrix   = GLresource<mat4>(shader, "worldMatrix");
    iTworldMatrix = GLresource<mat4>(shader, "iTworldMatrix");
    _color = GLresource<vec4>(shader, "tint");

    material.setShaders(shader, &worldMatrix, &iTworldMatrix, &_color);
    material.setTextures(tex);
}

void DrawMesh::draw(const mat4& world) {
    Renderable::draw(world);
    renderer->scheduleDrawElements(renderGroup, &vArray, &material, tesselPrim, _mesh->getRenderData()->ebuffer.size(), GLtype<uint32_t>());
}