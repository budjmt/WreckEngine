#include "DrawDebug.h"
#include <iostream>
DrawDebug::DrawDebug() {
	for (int i = 0; i < 4; i++)
		debugVectors.push_back(glm::vec3(0, 0, 0));

	vecShader = loadShaderProgram("Shaders/_debug/vecvertexShader.glsl", "Shaders/_debug/vecfragmentShader.glsl");
	meshShader = loadShaderProgram("Shaders/_debug/meshvertexShader.glsl", "Shaders/_debug/meshfragmentShader.glsl");
	sphere = loadOBJ("Assets/_debug/sphere.obj");
	if (sphere == nullptr) {
		genSphere("Assets/_debug/sphere.obj", 8);
		sphere = loadOBJ("Assets/_debug/sphere.obj");
	}

	glGenVertexArrays(1, &vecVAO);
	glBindVertexArray(vecVAO);
	glGenBuffers(1, &vecBuffer);
	
	glBindBuffer(GL_ARRAY_BUFFER, vecBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GL_FLOAT) * debugVectors.size() * FLOATS_PER_VERT, &debugVectors[0], GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, FLOATS_PER_VERT, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * FLOATS_PER_VERT * 2, 0);
	glVertexAttribPointer(1, FLOATS_PER_VERT, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * FLOATS_PER_VERT * 2, (void *)(sizeof(GL_FLOAT) * FLOATS_PER_VERT));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenVertexArrays(1, &arrowVAO);
	glBindVertexArray(arrowVAO);
	glGenBuffers(1, &arrowBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, arrowBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GL_FLOAT) * debugVectors.size() / 2 * 3 * FLOATS_PER_VERT, &debugVectors[0], GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, FLOATS_PER_VERT, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * FLOATS_PER_VERT * 2, 0);
	glVertexAttribPointer(1, FLOATS_PER_VERT, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * FLOATS_PER_VERT * 2, (void *)(sizeof(GL_FLOAT) * FLOATS_PER_VERT));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenVertexArrays(1, &meshVAO);
	glBindVertexArray(meshVAO);
	glGenBuffers(1, &sphereBuffer);
	glGenBuffers(1, &sphereElBuffer);

	glBindBuffer(GL_ARRAY_BUFFER, sphereBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GL_FLOAT) * sphere->verts().size() * FLOATS_PER_VERT, &(sphere->verts()[0]), GL_STATIC_DRAW);

	sphereVerts = sphere->faces().verts.size();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereElBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GL_UNSIGNED_INT) * sphereVerts, &(sphere->faces().verts[0]), GL_STATIC_DRAW);

	glVertexAttribPointer(0, FLOATS_PER_VERT, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * FLOATS_PER_VERT, 0);
	glEnableVertexAttribArray(0);

	vecCamLoc = glGetUniformLocation(vecShader, "cameraMatrix");
	meshCamLoc = glGetUniformLocation(meshShader, "cameraMatrix");
	worldLoc = glGetUniformLocation(meshShader, "worldMatrix");

	debugVectors = std::vector<glm::vec3>();
}

DrawDebug::~DrawDebug() {
	glDeleteBuffers(1, &vecBuffer);
	glDeleteBuffers(1, &arrowBuffer);
	glDeleteBuffers(1, &sphereBuffer);
}

DrawDebug& DrawDebug::getInstance() {
	static DrawDebug instance;
	return instance;
}

void DrawDebug::camera(Camera* c) { cam = c; }

void DrawDebug::draw() {	
	drawVectors();
	drawSpheres();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void DrawDebug::drawVectors() {
	for (int i = 0; i < 4; i++)
		debugVectors.push_back(glm::vec3(0, 0, 0));
	std::vector<glm::vec3> arrows;
	for (int i = 0; i < 6; i++)
		arrows.push_back(glm::vec3(0, 0, 0));
	int numVecs = debugVectors.size();
	for (int i = 0; i < numVecs; i++) {
		glm::vec3 s = debugVectors[i], c1 = debugVectors[i + 1], e = debugVectors[i + 2], c2 = debugVectors[i + 3];
		glm::vec3 v = e - s;
		v *= 0.05;
		v = e - v;
		arrows.push_back(v + glm::vec3(-1, 0, -1) * 0.008f);
		arrows.push_back(c1);

		arrows.push_back(v + glm::vec3(1, 0, 1)   * 0.008f);
		arrows.push_back(c1);

		arrows.push_back(e);
		arrows.push_back(c2);
	}

	glUseProgram(vecShader);
	if (cam != nullptr)
		cam->updateCamMat(vecCamLoc);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glBindVertexArray(vecVAO);
	glBindBuffer(GL_ARRAY_BUFFER, vecBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GL_FLOAT) * numVecs * FLOATS_PER_VERT, &(debugVectors[0]), GL_DYNAMIC_DRAW);
	glDrawArrays(GL_LINES, 0, numVecs / 2);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glBindVertexArray(arrowVAO);
	glBindBuffer(GL_ARRAY_BUFFER, arrowBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GL_FLOAT) * numVecs / 2 * 3 * FLOATS_PER_VERT, &(arrows[0]), GL_DYNAMIC_DRAW);
	glDrawArrays(GL_TRIANGLES, 0, numVecs / 4 * 3);

	debugVectors = std::vector<glm::vec3>();
}

void DrawDebug::drawSpheres() {
	glUseProgram(meshShader);
	if (cam != nullptr)
		cam->updateCamMat(meshCamLoc);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glBindVertexArray(meshVAO);
	glBindBuffer(GL_ARRAY_BUFFER, sphereBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereElBuffer);

	int numSpheres = debugSpheres.size();
	for (int i = 0; i < numSpheres; i++) {
		Sphere s = debugSpheres[i];
		glm::mat4 translate, scale;
		translate = glm::translate(s.center);
		scale = glm::scale(glm::vec3(1, 1, 1) * (s.rad * 2));
		glUniformMatrix4fv(worldLoc, 1, GL_FALSE, &(translate * scale)[0][0]);
		glDrawElements(GL_TRIANGLES, sphereVerts, GL_UNSIGNED_INT, (void *)0);
	}

	debugSpheres = std::vector<Sphere>();
}

void DrawDebug::drawDebugVector(glm::vec3 start, glm::vec3 end, glm::vec3 color) {
	debugVectors.push_back(start);
	debugVectors.push_back(color);
	debugVectors.push_back(end);
	debugVectors.push_back(color);
}

void DrawDebug::drawDebugSphere(glm::vec3 pos, float rad) {
	Sphere s = { pos, rad };
	debugSpheres.push_back(s);
	//drawDebugVector(pos, pos + glm::vec3(rad, 0, 0));
}