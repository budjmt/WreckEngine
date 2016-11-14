#include "DrawMesh.h"

DrawMesh::DrawMesh(shared<Mesh> m, const char* texFile, GLprogram shader) : _mesh(m) { setup(texFile, shader); }

DrawMesh::DrawMesh(Mesh::FaceData& fd, Mesh::FaceIndex& fi, const char* texFile, GLprogram shader) : DrawMesh(make_shared<Mesh>(fd, fi), texFile, shader) {}

void DrawMesh::setup(const char* texFile, GLprogram shader) {
	shaderProg = shader;
	
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

	textures.push_back(genTexture2D(texFile));
	textures[0].bind();

	// this needs to be refined at some point when multiple samplers come into the picture, probably with the material system
	shaderProg.use();
	textureLoc = _shaderProg.getUniform<GLsampler>("uniformTex");
	textureLoc.update(0);

	worldMatrix   = _shaderProg.getUniform<mat4>("worldMatrix");
	iTworldMatrix = _shaderProg.getUniform<mat4>("iTworldMatrix");
	colorLoc = _shaderProg.getUniform<vec4>("tint");
}

void DrawMesh::draw(const mat4& world) {
	Drawable::draw(world);
	textures[0].bind();
	GL_CHECK(glDrawElements(GL_TRIANGLES, _mesh->getRenderData()->ebuffer.size(), GL_UNSIGNED_INT, nullptr));
}