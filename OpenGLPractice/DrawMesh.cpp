#include "DrawMesh.h"

DrawMesh::DrawMesh(shared<Mesh> m, const char* texFile, GLprogram shader) : _mesh(m) { setup(texFile, shader); }

DrawMesh::DrawMesh(std::vector<vec3> v, std::vector<vec3> n, std::vector<vec3> u, Face f, const char* texFile, GLprogram shader) 
	: DrawMesh(make_shared<Mesh>(v, n, u, f), texFile, shader) {}

void DrawMesh::setup(const char* texFile, GLprogram shader) {
	shaderProg = shader;

	vArray.create();
	vArray.bind();

	vertBuffer.create(GL_ARRAY_BUFFER);
	vertBuffer.bind();
	vertBuffer.data(sizeof(GLfloat) * _mesh->meshArray.size(), &_mesh->meshArray[0]);

	elBuffer.create(GL_ELEMENT_ARRAY_BUFFER);
	elBuffer.bind();
	elBuffer.data(sizeof(GLuint) * _mesh->meshElementArray.size(), &_mesh->meshElementArray[0]);

	//set up an attribute for how the coordinates will be read
	GLattrarr attrSetup;
	attrSetup.add<GLfloat>(FLOATS_PER_VERT);
	attrSetup.add<GLfloat>(FLOATS_PER_UV);
	attrSetup.add<GLfloat>(FLOATS_PER_NORM);
	//enable attributes
	attrSetup.apply();

	textures.push_back(genTexture2D(texFile));
	textures[0].bind();

	textureLoc = _shaderProg.getUniform<GLsampler>("uniformTex");
	textureLoc.update(0);

	worldMatrix   = _shaderProg.getUniform<mat4>("worldMatrix");
	iTworldMatrix = _shaderProg.getUniform<mat4>("iTworldMatrix");
	colorLoc = _shaderProg.getUniform<vec4>("tint");

	glBindVertexArray(0);
}

void DrawMesh::draw(const mat4& world) {
	Drawable::draw(world);
	glDrawElements(GL_TRIANGLES, _mesh->meshElementArray.size(), GL_UNSIGNED_INT, nullptr);
}