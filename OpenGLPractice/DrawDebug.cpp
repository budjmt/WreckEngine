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
	glVertexAttribPointer(9, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * FLOATS_PER_VERT, 0);
	glEnableVertexAttribArray(9);

	glGenVertexArrays(1, &arrowVAO);
	glBindVertexArray(arrowVAO);
	glGenBuffers(1, &arrowBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, arrowBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GL_FLOAT) * debugVectors.size() / 2 * 3 * FLOATS_PER_VERT, &debugVectors[0], GL_DYNAMIC_DRAW);
	glVertexAttribPointer(10, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * FLOATS_PER_VERT, 0);
	glEnableVertexAttribArray(10);
	debugVectors = std::vector<glm::vec3>();
}

DrawDebug& DrawDebug::getInstance() {
	static DrawDebug instance;
	return instance;
}

void DrawDebug::draw() {	
	debugVectors.push_back(glm::vec3(1, 0, -1));
	debugVectors.push_back(glm::vec3(-1, 0, -1));
	std::vector<glm::vec3> arrows;
	arrows.push_back(glm::vec3(0, 0, 0));
	arrows.push_back(glm::vec3(0, 0, 0));
	arrows.push_back(glm::vec3(0, 0, 0));
	int numVecs = debugVectors.size();
	for (int i = 0; i < numVecs; i += 2) {
		glm::vec3 s = debugVectors[i], e = debugVectors[i + 1];
		arrows.push_back(e + glm::vec3(-0.2f, 0.2f, -0.2f));
		arrows.push_back(e + glm::vec3(0.2f, 0.2f, 0.2f));
		arrows.push_back(e + glm::vec3(0, -0.2f, 0));
	}
	glUseProgram(vecShader);
	glBindVertexArray(vecVAO);
	glBindBuffer(GL_ARRAY_BUFFER, vecBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GL_FLOAT) * numVecs * FLOATS_PER_VERT, &debugVectors[0], GL_DYNAMIC_DRAW);
	glDrawArrays(GL_LINES, 0, numVecs * FLOATS_PER_VERT);

	glBindVertexArray(arrowVAO);
	glBindBuffer(GL_ARRAY_BUFFER, arrowBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GL_FLOAT) * numVecs / 2 * 3 * FLOATS_PER_VERT, &arrows[0], GL_DYNAMIC_DRAW);
	glDrawArrays(GL_TRIANGLES, 0, numVecs / 2 * 3 * 3);

	debugVectors = std::vector<glm::vec3>();
}

void DrawDebug::drawDebugVector(glm::vec3 start, glm::vec3 end) {
	debugVectors.push_back(start);
	debugVectors.push_back(end);
}