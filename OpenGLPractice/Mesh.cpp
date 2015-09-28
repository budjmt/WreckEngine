#include "Mesh.h"
#include <iostream>

Mesh::Mesh(std::vector<GLfloat> v, std::vector<GLfloat> n, std::vector<GLfloat> u, Face f, char* texFile, GLuint shader)
	//: verts(mverts), normals(mnormals), tCoords(mtCoords), faces(mfaces)
{
	verts = v;
	normals = n;
	uvs = u;
	faces = f;
	shaderProg = shader;

	int numVerts = verts.size() / FLOATS_PER_VERT;
	int numUvs = uvs.size() / FLOATS_PER_UV;
	
	for (int i = 0; i < faces.verts.size(); i++) {
		//std::cout << i << std::endl;
		bool inArr = false;
		int index;
		for (index = 0; !inArr && index < faces.combined.size();index++) {
			if (faces.combined[index].x == faces.verts[i] && faces.combined[index].y == faces.uvs[i] && faces.combined[index].z == faces.normals[i]) {
				inArr = true;
				index--;
			}
		}
		if (!inArr) {
			faces.combined.push_back(glm::vec3(faces.verts[i], faces.uvs[i], faces.normals[i]));
			meshArray.push_back(verts[faces.verts[i] * FLOATS_PER_VERT]);
			meshArray.push_back(verts[faces.verts[i] * FLOATS_PER_VERT + 1]);
			meshArray.push_back(verts[faces.verts[i] * FLOATS_PER_VERT + 2]);
			meshArray.push_back(uvs[faces.uvs[i] * FLOATS_PER_UV]);
			meshArray.push_back(uvs[faces.uvs[i] * FLOATS_PER_UV + 1]);
			meshArray.push_back(normals[faces.normals[i] * FLOATS_PER_NORM]);
			meshArray.push_back(normals[faces.normals[i] * FLOATS_PER_NORM + 1]);
			meshArray.push_back(normals[faces.normals[i] * FLOATS_PER_NORM + 2]);
		}
		meshElementArray.push_back(index);
	}

	/*for (int i = 0; i < meshArray.size(); i += FLOATS_PER_VERT + FLOATS_PER_UV) {
		std::cout << i / (FLOATS_PER_VERT + FLOATS_PER_UV) << ": " << meshArray[i] << "," << meshArray[i + 1] << meshArray[i + 2] << " \\ "
			<< meshArray[i + FLOATS_PER_VERT] << "," << meshArray[i + 1 + FLOATS_PER_VERT] << std::endl;
	}
	std::cout << std::endl;

	int vertStride = FLOATS_PER_VERT;
	int totalStride = vertStride + FLOATS_PER_UV;
	for (int i = 0; i < meshElementArray.size(); i++) {
		std::cout << meshElementArray[i] << ": " << faces.combined[meshElementArray[i]].x << "/" << faces.combined[meshElementArray[i]].y << "/" << faces.combined[meshElementArray[i]].z << ": "
			<< meshArray[meshElementArray[i] * totalStride] << "," << meshArray[meshElementArray[i] * totalStride + 1] << "," << meshArray[meshElementArray[i] * totalStride + 2] << " \\ "
			<< meshArray[meshElementArray[i] * totalStride + vertStride] << "," << meshArray[meshElementArray[i] * totalStride + vertStride + 1] << std::endl;
		if ((i + 1) % 3 == 0)
			std::cout << std::endl;
	}*/

	//generate 1 vertex array at the address of the var
	//then make it active by binding it
	glGenVertexArrays(1, &vArray);
	glBindVertexArray(vArray);

	//generate 1 buffer at the address of the var
	//then make it active by binding it to the main buffer
	glGenBuffers(1, &vertBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertBuffer);
	//move the data in coords to the buffer and tell it how it'll be used
	glBufferData(GL_ARRAY_BUFFER, sizeof(GL_FLOAT) * meshArray.size(), &meshArray[0], GL_STATIC_DRAW);

	//generates the element array buffer for faces
	glGenBuffers(1, &vBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GL_UNSIGNED_INT) * meshElementArray.size(), &meshElementArray[0], GL_STATIC_DRAW);

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

	textures.push_back(SOIL_load_OGL_texture(texFile, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y));
	glBindTexture(GL_TEXTURE_2D, textures[0]);

	textureLoc = glGetUniformLocation(shader, "uniformTex");
	glUniform1ui(textureLoc, 0);

	//std::cout << vArray << "," << vertBuffer << "," << vBuffer << std::endl;

	worldMatrix = glGetUniformLocation(shaderProg, "worldMatrix");
	colorLoc = glGetUniformLocation(shaderProg, "tint");
}

Mesh::~Mesh()
{
	glDeleteBuffers(1, &vertBuffer);
	//delete[] meshArray;
	//delete[] meshElementArray;
}

void Mesh::draw(GLfloat x, GLfloat y, GLfloat xScale, GLfloat yScale) {
	Drawable::draw(x, y, xScale, yScale);
	glDrawElements(GL_TRIANGLES, faces.verts.size(), GL_UNSIGNED_INT, (void *)0);
}

void Mesh::draw(glm::vec3 pos, glm::vec3 scale, glm::vec3 rotAxis, float rot) {
	Drawable::draw(pos, scale, rotAxis, rot);
	glDrawElements(GL_TRIANGLES, meshElementArray.size(), GL_UNSIGNED_INT, (void *)0);
}