#include "DrawMesh.h"

DrawMesh::DrawMesh(Mesh* m, char* texFile, GLuint shader) {
	_mesh = m;
	setup(texFile, shader);
}

DrawMesh::DrawMesh(std::vector<glm::vec3> v, std::vector<glm::vec3> n, std::vector<glm::vec3> u, Face f, char* texFile, GLuint shader)
{
	_mesh = new Mesh(v, n, u, f);
	setup(texFile, shader);
}

void DrawMesh::setup(char* texFile, GLuint shader) {
	shaderProg = shader;

	//generate 1 vertex array at the address of the var
	//then make it active by binding it
	glGenVertexArrays(1, &vArray);
	glBindVertexArray(vArray);

	//generate 1 buffer at the address of the var
	//then make it active by binding it to the main buffer
	glGenBuffers(1, &vertBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertBuffer);
	//move the data in coords to the buffer and tell it how it'll be used
	glBufferData(GL_ARRAY_BUFFER, sizeof(GL_FLOAT) * _mesh->meshArray.size(), &(_mesh->meshArray[0]), GL_STATIC_DRAW);

	//generates the element array buffer for faces
	glGenBuffers(1, &vBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GL_UNSIGNED_INT) * _mesh->meshElementArray.size(), &(_mesh->meshElementArray[0]), GL_STATIC_DRAW);

	//set up an attribute for how the coordinates will be read
	//verts
	glVertexAttribPointer(0, FLOATS_PER_VERT, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * (FLOATS_PER_VERT + FLOATS_PER_UV + FLOATS_PER_NORM), 0);
	//uvs
	glVertexAttribPointer(1, FLOATS_PER_UV, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * (FLOATS_PER_VERT + FLOATS_PER_UV + FLOATS_PER_NORM), (void *)(sizeof(GL_FLOAT) * FLOATS_PER_VERT));
	//normals
	glVertexAttribPointer(2, FLOATS_PER_NORM, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * (FLOATS_PER_VERT + FLOATS_PER_UV + FLOATS_PER_NORM), (void *)(sizeof(GL_FLOAT) * (FLOATS_PER_VERT + FLOATS_PER_UV)));

	//enable that attribute at 0
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	GLuint texture = genTexture(texFile);

	textures.push_back(texture);
	glBindTexture(GL_TEXTURE_2D, textures[0]);

	textureLoc = glGetUniformLocation(shader, "uniformTex");
	glUniform1ui(textureLoc, 0);

	//std::cout << vArray << "," << vertBuffer << "," << vBuffer << std::endl;

	worldMatrix = glGetUniformLocation(shaderProg, "worldMatrix");
	iTworldMatrix = glGetUniformLocation(shaderProg, "iTworldMatrix");
	colorLoc = glGetUniformLocation(shaderProg, "tint");

	glBindVertexArray(0);
}

DrawMesh::~DrawMesh()
{
	glDeleteBuffers(1, &vertBuffer);
	delete _mesh;
}

Mesh* DrawMesh::mesh() const { return _mesh; } void DrawMesh::mesh(Mesh* m) { _mesh = m; }

void DrawMesh::draw(GLfloat x, GLfloat y, GLfloat xScale, GLfloat yScale) {
	Drawable::draw(x, y, xScale, yScale);
	glDrawElements(GL_TRIANGLES, _mesh->meshElementArray.size(), GL_UNSIGNED_INT, (void *)0);
}

void DrawMesh::draw(glm::vec3 pos, glm::vec3 scale, glm::vec3 rotAxis, float rot) {
	Drawable::draw(pos, scale, rotAxis, rot);
	glDrawElements(GL_TRIANGLES, _mesh->meshElementArray.size(), GL_UNSIGNED_INT, (void *)0);
}