#include "Shape.h"

Shape::Shape(GLfloat* verts, GLint numV, GLuint shader)
{
	numVerts = numV;
	shaderProg = shader;

	//generate 1 vertex array at the address of the var
	//then make it active by binding it
	glGenVertexArrays(1, &vArray);
	glBindVertexArray(vArray);

	//generate 1 buffer at the address of the var
	//then make it active by binding it to the main buffer
	glGenBuffers(1, &vBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vBuffer);

	//move the data in coords to the buffer and tell it how it'll be used
	glBufferData(GL_ARRAY_BUFFER, sizeof(GL_FLOAT) * numVerts * FLOATS_PER_VERT, verts, GL_STATIC_DRAW);

	//set up an attribute for how the coordinates will be read at 0
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * FLOATS_PER_VERT, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * FLOATS_PER_VERT, (void*)(sizeof(GL_FLOAT) * 2));
	
	//enable that attribute at 0
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	worldMatrix = glGetUniformLocation(shaderProg,"worldMatrix");
	colorLoc = glGetUniformLocation(shaderProg, "tint");
}

void Shape::draw(GLfloat x, GLfloat y, GLfloat xScale, GLfloat yScale) {
	
	Drawable::draw(x, y, xScale, yScale);

	//GL_LINE_STRIP draws a line from point to point
	//GL_LINE_LOOP does the same but connects the last point to the first
	glDrawArrays(GL_TRIANGLE_STRIP,0,numVerts);
}

void Shape::draw(glm::vec3 pos, glm::vec3 scale, glm::vec3 rotAxis, float rot) {
	
	Drawable::draw(pos, scale, rotAxis, rot);

	//GL_LINE_STRIP draws a line from point to point
	//GL_LINE_LOOP does the same but connects the last point to the first
	//GL_TRIANGLE_FAN draws each vertex as a triangle with the next vertex and the first one.
	//GL_TRIANGLE_STRIP draws every three vertices as a triangle
	glDrawArrays(GL_TRIANGLE_STRIP,0,numVerts);
}
