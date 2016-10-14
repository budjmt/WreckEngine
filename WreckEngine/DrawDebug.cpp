#include "DrawDebug.h"
#include <iostream>

DrawDebug::DrawDebug() {
#if DEBUG
	vecShader  = loadProgram("Shaders/_debug/vecvertexShader.glsl", "Shaders/_debug/vecfragmentShader.glsl");
	meshShader = loadProgram("Shaders/_debug/meshvertexShader.glsl", "Shaders/_debug/meshfragmentShader.glsl");
	
	arrow = loadOBJ("Assets/_debug/arrow.obj");
	if (!arrow) {
		genCone("Assets/_debug/arrow.obj", 8);
		arrow = loadOBJ("Assets/_debug/arrow.obj");
	}
	
	sphere = loadOBJ("Assets/_debug/sphere.obj");
	if (!sphere) {
		genSphere("Assets/_debug/sphere.obj", 8);
		sphere = loadOBJ("Assets/_debug/sphere.obj");
	}

	box = loadOBJ("Assets/_debug/cube.obj");
	if (!box) {
		genCube("Assets/_debug/cube.obj");
		box = loadOBJ("Assets/_debug/cube.obj");
	}

	GLattrarr attrSetup;
	const auto vecSize = sizeof(GLfloat) * FLOATS_PER_VERT;

	vecVAO.create();
	vecVAO.bind();
	vecBuffer.create(GL_ARRAY_BUFFER, GL_STREAM_DRAW);

	vecBuffer.bind();
	vecBuffer.data(vecSize * MAX_VECTORS * 4, nullptr);
	debugVectors.reserve(MAX_VECTORS * 4);

	attrSetup.add<vec3>(2);
	attrSetup.apply();

	const auto mSetup = [](GLattrarr& attrSetup) {
		attrSetup.add<vec4>(1, 1);
		attrSetup.add<mat4>(1, 1);
	};

	arrows = InstMesh<m_MeshData>(arrow.get(), MAX_VECTORS, 1, mSetup);
	spheres = InstMesh<m_MeshData>(sphere.get(), MAX_SPHERES, 1, mSetup);
	boxes = InstMesh<m_MeshData>(box.get(), MAX_BOXES, 1, mSetup);

	vecVAO.unbind();

	vecCamLoc  = vecShader.getUniform<mat4>("cameraMatrix");
	meshCamLoc = meshShader.getUniform<mat4>("cameraMatrix");
#endif
}

DrawDebug& DrawDebug::getInstance() {
	static DrawDebug instance;
	return instance;
}

void DrawDebug::camera(Camera* c) { cam = c; }

void DrawDebug::draw() {
#if DEBUG
	glEnable(GL_CULL_FACE);
	
	drawVectors();
	drawSpheres();
	drawBoxes();
	
	glDisable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif
}

void DrawDebug::drawVectors() {
	if (!debugVectors.size()) {
		for (auto i = 0; i < 4; ++i) debugVectors.push_back(vec3());
	}

	const auto numVecs = debugVectors.size();
	for (size_t i = 0; i < numVecs; i += 4) {
		const auto sfact = 0.05f;
		const auto s = debugVectors[i]    , c1 = debugVectors[i + 1]
		         , e = debugVectors[i + 2], c2 = debugVectors[i + 3];
		
		const auto es = e - s;
		const auto v = e - es * (sfact * 0.5f);

		const auto translate = glm::translate(v);
		const auto rotate = rotateBetween(vec3(0,1,0), glm::normalize(es));
		const auto scale = glm::scale(vec3(0.5f, 1.f, 0.5f) * sfact);
		arrows.instances.push_back({ vec4(c2, 1), translate * rotate * scale });
	}

	vecShader.use();
	if (cam) cam->updateCamMat(vecCamLoc);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	vecVAO.bind();
	vecBuffer.bind();
	vecBuffer.data(&debugVectors[0]);
	glDrawArrays(GL_LINES, 0, numVecs / 2);

	meshShader.use();
	if (cam) cam->updateCamMat(meshCamLoc);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	arrows.bind();
	arrows.update();
	arrows.draw();

	debugVectors.clear();
	arrows.instances.clear();
}

void DrawDebug::drawSpheres() {	
	if (!debugSpheres.size()) debugSpheres.push_back(Sphere());
	
	for (const auto& s : debugSpheres) {
		const auto translate = glm::translate(s.center);
		const auto scale = glm::scale(vec3(s.rad * 2));
		spheres.instances.push_back({ s.color, translate * scale });
	}

	meshShader.use();
	if (cam) cam->updateCamMat(meshCamLoc);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	spheres.bind();
	spheres.update();
	spheres.draw();

	debugSpheres = std::vector<Sphere>();
	spheres.instances.clear();
}

void DrawDebug::drawBoxes() {
	if (!debugBoxes.size()) for(auto i = 0; i < 3; ++i) debugBoxes.push_back(vec4());

	for (size_t i = 0, numBoxes = debugBoxes.size(); i < numBoxes; i += 3) {
		const auto translate = glm::translate(vec3(debugBoxes[i]));
		const auto scale = glm::scale(vec3(debugBoxes[i + 1]));
		boxes.instances.push_back({ debugBoxes[i + 2], translate * scale });
	}

	meshShader.use();
	if (cam) cam->updateCamMat(meshCamLoc);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	boxes.bind();
	boxes.update();
	boxes.draw();

	debugBoxes = std::vector<vec4>();
	boxes.instances.clear();
}

void DrawDebug::drawDebugVector(vec3 start, vec3 end, vec3 color) {
#if DEBUG
	if (debugVectors.size() > MAX_VECTORS * 4) return;
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
#endif
}

void DrawDebug::drawDebugBox(vec3 pos, float w, float h, float d, vec3 color, float opacity) {
#if DEBUG
	if (debugBoxes.size() > MAX_BOXES * 2) return;
	debugBoxes.push_back(vec4(pos, 0));
	debugBoxes.push_back(vec4(w, h, d, 0));
	debugBoxes.push_back(vec4(color, opacity));
#endif
}