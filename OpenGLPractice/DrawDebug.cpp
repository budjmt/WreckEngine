#include "DrawDebug.h"
#include <iostream>
DrawDebug::DrawDebug() {
	debugVectors.push_back(glm::vec3(0, 0, 0));
	debugVectors.push_back(glm::vec3(0, 0, 0));
	vecShader = loadShaderProgram("Shaders/debugVectorVertexShader.glsl", "Shaders/debugVectorFragmentShader.glsl");
	glGenVertexArrays(1, &vecVAO);
	glBindVertexArray(vecVAO);
	glGenBuffers(1, &vecBuffer);
	
	glBindBuffer(GL_ARRAY_BUFFER, vecBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GL_FLOAT) * debugVectors.size() * FLOATS_PER_VERT, &debugVectors[0], GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * FLOATS_PER_VERT, 0);
	glEnableVertexAttribArray(0);

	glGenVertexArrays(1, &arrowVAO);
	glBindVertexArray(arrowVAO);
	glGenBuffers(1, &arrowBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, arrowBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GL_FLOAT) * debugVectors.size() / 2 * 3 * FLOATS_PER_VERT, &debugVectors[0], GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * FLOATS_PER_VERT, 0);
	glEnableVertexAttribArray(0);
	debugVectors = std::vector<glm::vec3>();
}

DrawDebug& DrawDebug::getInstance() {
	static DrawDebug instance;
	return instance;
}

void DrawDebug::camera(Camera* c) {
	cam = c;
	camLoc = glGetUniformLocation(vecShader, "cameraMatrix");
}

void DrawDebug::draw() {	
	debugVectors.push_back(glm::vec3(0, 0, 0));
	debugVectors.push_back(glm::vec3(0, 0, 0));
	std::vector<glm::vec3> arrows;
	arrows.push_back(glm::vec3(0, 0, 0));
	arrows.push_back(glm::vec3(0, 0, 0));
	arrows.push_back(glm::vec3(0, 0, 0));
	int numVecs = debugVectors.size();
	for (int i = 0; i < numVecs; i += 2) {
		glm::vec3 s = debugVectors[i], e = debugVectors[i + 1];
		arrows.push_back(e + glm::vec3(-1, 2, -1) * 0.008f);
		arrows.push_back(e + glm::vec3(1, 2, 1)   * 0.008f);
		arrows.push_back(e);
	}
	glUseProgram(vecShader);
	if (cam != nullptr)
		cam->updateCamMat(camLoc);

	glPointSize(10);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	glBindVertexArray(vecVAO);
	glBindBuffer(GL_ARRAY_BUFFER, vecBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GL_FLOAT) * numVecs * FLOATS_PER_VERT, &(debugVectors[0]), GL_DYNAMIC_DRAW);
	glDrawArrays(GL_LINES, 0, numVecs);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glBindVertexArray(arrowVAO);
	glBindBuffer(GL_ARRAY_BUFFER, arrowBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GL_FLOAT) * numVecs / 2 * 3 * FLOATS_PER_VERT, &(arrows[0]), GL_DYNAMIC_DRAW);
	glDrawArrays(GL_TRIANGLES, 0, numVecs / 2 * 3);

	debugVectors = std::vector<glm::vec3>();
}

void DrawDebug::drawDebugVector(glm::vec3 start, glm::vec3 end) {
	debugVectors.push_back(start);
	debugVectors.push_back(end);
}