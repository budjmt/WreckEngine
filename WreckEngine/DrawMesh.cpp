#include "DrawMesh.h"

DrawMesh::DrawMesh(Render::MaterialRenderer* r, shared<Mesh> m, const char* texFile, GLprogram shader) : _mesh(m) { renderer = r; setup(texFile, shader); }

DrawMesh::DrawMesh(Render::MaterialRenderer* r, Mesh::FaceData& fd, Mesh::FaceIndex& fi, const char* texFile, GLprogram shader) : DrawMesh(r, make_shared<Mesh>(fd, fi), texFile, shader) {}

void DrawMesh::setup(const char* texFile, GLprogram shader) {
	
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

	// this needs to be refined at some point when multiple samplers come into the picture, probably with the material system
	shader.use();
	shader.setOnce<GLsampler>("uniformTex", 0);

	worldMatrix   = GLresource<mat4>(shader, "worldMatrix");
	iTworldMatrix = GLresource<mat4>(shader, "iTworldMatrix");;
	_color = GLresource<vec4>(shader, "tint");

    material.setShaders(shader, &worldMatrix, &iTworldMatrix, &_color);
    material.setTextures(genTexture2D(texFile));
}

void DrawMesh::draw(const mat4& world) {
	Drawable::draw(world);
    renderer->scheduleDrawElements(0, &vArray, &material, GL_TRIANGLES, _mesh->getRenderData()->ebuffer.size(), GLtype<uint32_t>());
}