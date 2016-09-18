#include "DrawDebug.h"
#include <iostream>
DrawDebug::DrawDebug() {
#if DEBUG
	vecShader  = loadGLProgram("Shaders/_debug/vecvertexShader.glsl", "Shaders/_debug/vecfragmentShader.glsl");
	meshShader = loadGLProgram("Shaders/_debug/meshvertexShader.glsl", "Shaders/_debug/meshfragmentShader.glsl");
	sphere = loadOBJ("Assets/_debug/sphere.obj");
	if (!sphere) {
		genSphere("Assets/_debug/sphere.obj", 8);
		sphere = loadOBJ("Assets/_debug/sphere.obj");
	}

	GLattrarr attrSetup;
	auto vecSize = sizeof(GLfloat) * FLOATS_PER_VERT;

	vecVAO.create();
	vecVAO.bind();
	vecBuffer.create(GL_ARRAY_BUFFER, GL_STREAM_DRAW);

	vecBuffer.bind();
	vecBuffer.data(vecSize * MAX_VECTORS * 4, nullptr);

	attrSetup.add<vec3>(2);
	attrSetup.apply();

	arrowVAO.create();
	arrowVAO.bind();
	arrowBuffer.create(GL_ARRAY_BUFFER, GL_STREAM_DRAW);
	arrowBuffer.bind();
	arrowBuffer.data(vecSize * MAX_VECTORS * 6, nullptr);

	attrSetup.add<vec3>(2);
	attrSetup.apply();

	sphereVAO.create();
	sphereVAO.bind();
	sphereBuffer.create(GL_ARRAY_BUFFER);
	sphereInstBuffer.create(GL_ARRAY_BUFFER, GL_STREAM_DRAW);
	sphereElBuffer.create(GL_ELEMENT_ARRAY_BUFFER);

	sphereElBuffer.bind();
	numSphereVerts = sphere->faces().verts.size();
	sphereElBuffer.data(sizeof(GLuint) * numSphereVerts, &sphere->faces().verts[0]);

	sphereBuffer.bind();
	sphereBuffer.data(vecSize * sphere->verts().size(), &sphere->verts()[0]);

	attrSetup.add<vec3>(1);
	attrSetup.apply();

	sphereInstBuffer.bind();
	sphereInstBuffer.data(sizeof(m_MeshData) * MAX_SPHERES, nullptr);

	attrSetup.add<vec4>(1, 1);
	attrSetup.add<mat4>(1, 1);
	attrSetup.apply(1);

	glBindVertexArray(0);

	vecCamLoc  = vecShader.getUniform<mat4>("cameraMatrix");
	meshCamLoc = meshShader.getUniform<mat4>("cameraMatrix");

	debugVectors.reserve(MAX_VECTORS * 4);
	arrows.reserve(MAX_VECTORS * 6);
	sphereInsts.reserve(MAX_SPHERES);
#endif
}

DrawDebug& DrawDebug::getInstance() {
	static DrawDebug instance;
	return instance;
}

void DrawDebug::camera(Camera* c) { cam = c; }

void DrawDebug::draw() {
#if DEBUG
	drawVectors();

	glEnable(GL_CULL_FACE);
	drawSpheres();
	glDisable(GL_CULL_FACE);
	
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif
}

void DrawDebug::drawVectors() {
	if (!debugVectors.size()) {
		for (auto i = 0; i < 4; ++i) debugVectors.push_back(vec3());
	}

	auto numVecs = debugVectors.size();
	for (size_t i = 0; i < numVecs; i += 4) {
		vec3 s = debugVectors[i]    , c1 = debugVectors[i + 1]
		   , e = debugVectors[i + 2], c2 = debugVectors[i + 3];
		
		auto v = e - (e - s) * 0.05f;
		arrows.push_back(v + vec3(-1, 0, -1) * 0.008f);
		arrows.push_back(c1);

		arrows.push_back(v + vec3(1, 0, 1) * 0.008f);
		arrows.push_back(c1);

		arrows.push_back(e);
		arrows.push_back(c2);
	}

	vecShader.use();
	if (cam) cam->updateCamMat(vecCamLoc);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	vecVAO.bind();
	vecBuffer.bind();
	vecBuffer.data(&debugVectors[0]);
	glDrawArrays(GL_LINES, 0, numVecs / 2);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	arrowVAO.bind();
	arrowBuffer.bind();
	arrowBuffer.data(&arrows[0]);
	glDrawArrays(GL_TRIANGLES, 0, numVecs / 2 * 3);

	debugVectors.clear();
	arrows.clear();
}

void DrawDebug::drawSpheres() {	
	if (!debugSpheres.size()) debugSpheres.push_back(Sphere());
	
	for (auto& s : debugSpheres) {
		auto translate = glm::translate(s.center);
		auto scale = glm::scale(vec3(1) * (s.rad * 2));
		sphereInsts.push_back({ s.color, translate * scale });
	}

	meshShader.use();
	if (cam) cam->updateCamMat(meshCamLoc);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	sphereVAO.bind();
	sphereInstBuffer.bind();
	sphereInstBuffer.data(&sphereInsts[0]);
	glDrawElementsInstanced(GL_TRIANGLES, numSphereVerts, GL_UNSIGNED_INT, nullptr, debugSpheres.size());

	debugSpheres = std::vector<Sphere>();
	sphereInsts.clear();
}

void DrawDebug::drawDebugVector(vec3 start, vec3 end, vec3 color) {
#if DEBUG
	if (debugVectors.size() / 4 > MAX_VECTORS) return;
	debugVectors.push_back(start);
	debugVectors.push_back(color);
	debugVectors.push_back(end);
	debugVectors.push_back(color);
#endif
}

void DrawDebug::drawDebugSphere(vec3 pos, float rad, vec3 color, float opacity) {
#if DEBUG
	if (debugSpheres.size() > MAX_SPHERES) return;
	debugSpheres.push_back({ vec4(color, opacity), pos, rad });
	//drawDebugVector(pos, pos + vec3(rad, 0, 0));
#endif
}