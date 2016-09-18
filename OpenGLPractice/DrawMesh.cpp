#include "DrawMesh.h"

DrawMesh::DrawMesh(shared<Mesh> m, const char* texFile, GLprogram shader) : _mesh(m) { setup(texFile, shader); }

DrawMesh::DrawMesh(std::vector<vec3> v, std::vector<vec3> n, std::vector<vec3> u, Face f, const char* texFile, GLprogram shader) 
	: DrawMesh(make_shared<Mesh>(v, n, u, f), texFile, shader) {}

void DrawMesh::setup(const char* texFile, GLprogram shader) {
	shaderProg = shader;

	//generate 1 vertex array at the address of the var
	//then make it active by binding it
	vArray.create();
	vArray.bind();

	//generate 1 buffer at the address of the var
	//then make it active by binding it to the main buffer
	vertBuffer.create(GL_ARRAY_BUFFER);
	vertBuffer.bind();
	//move the data in coords to the buffer and tell it how it'll be used
	vertBuffer.data(sizeof(GLfloat) * _mesh->meshArray.size(), &_mesh->meshArray[0]);

	//generates the element array buffer for faces
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

	textures.push_back(genTexture(texFile));
	glBindTexture(GL_TEXTURE_2D, textures[0]);

	textureLoc = _shaderProg.getUniform<GLsampler>("uniformTex");
	textureLoc.update(0);

	//std::cout << vArray << "," << vertBuffer << "," << vBuffer << std::endl;

	worldMatrix   = _shaderProg.getUniform<mat4>("worldMatrix");
	iTworldMatrix = _shaderProg.getUniform<mat4>("iTworldMatrix");
	colorLoc = _shaderProg.getUniform<vec4>("tint");

	glBindVertexArray(0);
}

Mesh* DrawMesh::mesh() const { return _mesh.get(); } void DrawMesh::mesh(Mesh* m) { _mesh.reset(m); }

void DrawMesh::draw(GLfloat x, GLfloat y, GLfloat xScale, GLfloat yScale) {
	Drawable::draw(x, y, xScale, yScale);
	glDrawElements(GL_TRIANGLES, _mesh->meshElementArray.size(), GL_UNSIGNED_INT, nullptr);
}

void DrawMesh::draw(vec3 pos, vec3 scale, vec3 rotAxis, float rot) {
	Drawable::draw(pos, scale, rotAxis, rot);
	glDrawElements(GL_TRIANGLES, _mesh->meshElementArray.size(), GL_UNSIGNED_INT, nullptr);
}