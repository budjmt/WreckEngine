#include "TriPlay.h"

#include <iostream>

TriPlay::TriPlay()
{
}

TriPlay::TriPlay(GLuint prog, GLFWwindow* w)
	: Game(prog)
{
	window = w;
	int width, height;
	glfwGetWindowSize(window, &width, &height);

	/*GLfloat triVerts[3 * FLOATS_PER_VERT] = {
	-0.75f, -0.75f, 1, 1, 1,
	0.75f, -0.75f, 1, 1, 1,
	0, 0.75f, 1, 1, 1
	};
	Shape* triShape = new Shape(triVerts, 3, prog);
	shapes.push_back(triShape);

	GLfloat baseVerts[4 * FLOATS_PER_VERT] = {
	-1, 0.25, 0, 0, 0,
	1, 0.25, 0, 0, 0,
	1, -0.25, 0, 0, 0,
	-1, -0.25, 0, 0, 0
	};
	Shape* baseShape = new Shape(baseVerts, 4, prog);
	shapes.push_back(baseShape);

	triangles = std::vector<ColliderEntity*>();
	for (int i = 0; i < 100; i++) {
	ColliderEntity* triangle = new ColliderEntity(glm::vec3(0, 0, 0), glm::vec3(0.2f * 0.75f * width, 0.2f * 0.75f * height, 0), glm::vec3(0.2f, 0.2f, 0.2f), glm::vec3(0, 0, 1), 0, triShape);
	triangle->active = false;
	triangle->collider->type = CIRCLE;
	//triangle->setColor(glm::vec4(glm::circularRand(1.f),glm::circularRand(1.f)));//this won't work because the triangles are all batched
	triangle->color = glm::vec4(0.5f,0,0,1);
	triangles.push_back(triangle);
	}

	ColliderEntity* base = new ColliderEntity(glm::vec3(width / 2, height + 20, 0), glm::vec3(width,0.1f * 0.25f * height,0), glm::vec3(1, 0.1f, 1), glm::vec3(0, 0, 1), 0, baseShape);
	base->staticObj = 1;
	base->collider->type = RECT;
	entities.push_back(base);
	*/
	Mesh* m = loadOBJ("Assets/basic.obj");
	DrawMesh* dm = new DrawMesh(m, "Assets/texture.png", prog);
	shapes.push_back(dm);
	Entity* mesh = new ColliderEntity(dm);
	mesh->transform.rotAxis = glm::vec3(0, 1, 0);
	mesh->transform.position.x = 0;
	entities.push_back(mesh);
	meshes.push_back(mesh);
	//me = mesh;

	std::vector<std::vector<glm::vec3>> k = {
		{ glm::vec3( 1,-1,-1), glm::vec3( 1,-1, 1), glm::vec3( 1, 1, 1) },
		{ glm::vec3(-1, 1,-1), glm::vec3(-1, 1, 1), glm::vec3(-1,-1, 1) },
		{ glm::vec3(-1,-1, 1), glm::vec3(-1,-1,-1), glm::vec3(-1, 1,-1) },
		{ glm::vec3( 1, 1,-1), glm::vec3( 1, 1, 1), glm::vec3( 1,-1, 1) }
	};
	/*std::vector<std::vector<glm::vec3>> k = {
		{ glm::vec3( 1, 1, 0), glm::vec3(-1, 1, 0) },
		{ glm::vec3(1,-1, 0), glm::vec3(-1,-1, 0) }
	};*/
	//genBezierSurface("Assets/bezier.obj",16,16,k);
	Mesh* bezier = loadOBJ("Assets/bezier.obj");
	dm = new DrawMesh(bezier, "Assets/texture.png", prog);
	shapes.push_back(dm);
	mesh = new ColliderEntity(dm);
	mesh->transform.rotAxis = glm::vec3(0, 1, 0);
	mesh->transform.position.x = 2.5f;
	entities.push_back(mesh);
	meshes.push_back(mesh);

	//genCylinder("Assets/cylinder.obj", 64);
	Mesh* cylinder = loadOBJ("Assets/cylinder.obj");
	dm = new DrawMesh(cylinder, "Assets/texture.png", prog);
	shapes.push_back(dm);
	mesh = new ColliderEntity(dm);
	mesh->transform.rotAxis = glm::vec3(0, 1, 0);
	mesh->transform.position.x = -2.5f;
	entities.push_back(mesh);
	meshes.push_back(mesh);

	//genSphere("Assets/sphere.obj", 16);
	Mesh* sphere = loadOBJ("Assets/sphere.obj");
	dm = new DrawMesh(sphere, "Assets/texture.png", prog);
	shapes.push_back(dm);
	mesh = new ColliderEntity(dm);
	mesh->transform.rotAxis = glm::vec3(0, 1, 0);
	mesh->transform.position.y = 2.5f;
	entities.push_back(mesh);
	meshes.push_back(mesh);

	//genCube("Assets/cube.obj");
	Mesh* cube = loadOBJ("Assets/cube.obj");
	dm = new DrawMesh(cube, "Assets/texture.png", prog);
	shapes.push_back(dm);
	mesh = new ColliderEntity(dm);
	mesh->transform.rotAxis = glm::vec3(0, 1, 0);
	mesh->transform.position.y = -2.5f;
	entities.push_back(mesh);
	meshes.push_back(mesh);

	dm = new DrawMesh(cube, "Assets/texture.png", prog);
	shapes.push_back(dm);
	mesh = new ColliderEntity(dm);
	mesh->transform.rotAxis = glm::vec3(0, 1, 0);
	mesh->transform.position.y = -2.5f;
	mesh->transform.position.x = -2.5f;
	entities.push_back(mesh);
	meshes.push_back(mesh);
	me = mesh;

	camera = new Camera(prog, window);
	camera->transform.position = glm::vec3(0, 0, 1);
	camera->transform.rotate(0, M_PI, 0);
	entities.push_back(camera);
}

TriPlay::TriPlay(const TriPlay& other)
	: Game(other)
{
}

TriPlay::~TriPlay()
{
	/*while (triangles.size()) {
	delete triangles[0];
	triangles.pop_back();
	}*/
	while (meshes.size()) {
		delete meshes[0];
		meshes.pop_back();
	}
}

void TriPlay::update(GLFWwindow* window, Mouse* m, double dt) {
	Game::update(dt);
	/*if (m->down
	&& m->lastClick + m->clickCoolDown < prevFrame + dt) {
	glm::vec3 pos, vel;
	pos = glm::vec3(m->x, m->y, 0);
	vel = glm::normalize(pos - glm::vec3(m->prevx, m->prevy, 0));
	//vel /= dt * 850;//on the lab computers 850 makes it appear that velocity isn't working because it's too slow. just comment this line out if on one
	spawnTriangle(pos,vel);
	m->lastClick = prevFrame + dt;
	}*/

	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
		//quit the game
		glfwTerminate();
		exit('q');
	}

	bool shift = false, ctrl = false;
	if (glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		shift = true;
	if (glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		ctrl = true;

	if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) {
		if(shift)
			me->transform.position += glm::vec3(0, 0, dt);
		else if(ctrl)
			me->transform.rotate(2 * M_PI * dt, 0, 0);
		else
			me->transform.position += glm::vec3(0, dt, 0); 
	}
	else if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
		if(shift)
			me->transform.position += glm::vec3(0, 0, -dt);
		else if (ctrl)
			me->transform.rotate(-2 * M_PI * dt, 0, 0); 
		else
			me->transform.position += glm::vec3(0, -dt, 0); 
	}
	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
		if (shift)
			me->transform.rotate(0, 2 * M_PI * dt, 0);
		else if (ctrl)
			me->transform.rotate(0, 0, 2 * M_PI * dt);
		else
			me->transform.position += glm::vec3(dt, 0, 0); 
	}
	else if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
		if (shift)
			me->transform.rotate(0, -2 * M_PI * dt, 0);
		else if (ctrl)
			me->transform.rotate(0, 0, -2 * M_PI * dt);
		else
			me->transform.position += glm::vec3(-dt, 0, 0); 
	}

	Camera::mayaCam(window, m, dt, camera);

	//edges, gauss map, and normals confirmed to be ok
	/*Collider* c = ((ColliderEntity*)me)->collider();
	//int e[2] = { 1,2 };
	//glm::vec3 edge = c->getEdge(e);
	glm::vec3 norm = c->getCurrNormals()[0];
	std::cout << norm.x << ", " << norm.y << ", " << norm.z << std::endl;*/
}

/*void TriPlay::spawnTriangle(glm::vec3 pos, glm::vec3 vel) {
if (!triangles.size())
return;
ColliderEntity* end = triangles[triangles.size() - 1];
end->pos = pos;
end->vel = vel;
end->angVel = glm::vec3(glm::circularRand(1.f) * 0.001f,0) * glm::sign(vel.x);
end->rot = glm::linearRand(0.f,glm::two_pi<float>());
end->active = true;
entities.push_back(end);
triangles.pop_back();
}*/