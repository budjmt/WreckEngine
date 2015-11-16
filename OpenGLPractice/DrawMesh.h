#pragma once

#include "Drawable.h"
#include "Mesh.h"

class DrawMesh : public Drawable
{
public:
	DrawMesh(Mesh* m, char* texFile, GLuint shader);
	DrawMesh(std::vector<glm::vec3> v, std::vector<glm::vec3> n, std::vector<glm::vec3> u, Face f, char* texFile, GLuint shader);
	~DrawMesh();
	void setup(char* texFile, GLuint shader);
	void draw(GLfloat x, GLfloat y, GLfloat xScale, GLfloat yScale);
	void draw(glm::vec3 pos, glm::vec3 scale, glm::vec3 rotAxis, float rot);
private:
	GLuint vertBuffer;
	GLuint textureLoc;
	std::vector<GLuint> textures;
	Mesh* mesh;
};

