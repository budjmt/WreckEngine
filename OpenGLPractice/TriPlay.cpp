#include "TriPlay.h"

#include <iostream>

TriPlay::TriPlay(GLprogram prog, GLFWwindow* w) : Game(prog), window(w)
{
	int width, height;
	glfwGetWindowSize(window, &width, &height);

	auto mainState = make_shared<State>("main");
	mainState->handler_func = [](Event e) {
		//nothing right now
	};
	addState(mainState);

	auto m = loadOBJ("Assets/basic.obj");
	m->translateTo(vec3());
	auto dm = make_shared<DrawMesh>(m, "Assets/texture.png", prog);
	auto mesh = make_shared<ColliderEntity>(dm);
	mesh->id = (void*)0xcaca;
	mainState->addEntity(mesh);
	//me = mesh;

	std::vector<std::vector<vec3>> k = {
		{ vec3( 1,-1,-1), vec3( 1,-1, 1), vec3( 1, 1, 1) },
		{ vec3(-1, 1,-1), vec3(-1, 1, 1), vec3(-1,-1, 1) },
		{ vec3(-1,-1, 1), vec3(-1,-1,-1), vec3(-1, 1,-1) },
		{ vec3( 1, 1,-1), vec3( 1, 1, 1), vec3( 1,-1, 1) }
	};
	/*std::vector<std::vector<vec3>> k = {
		{ vec3( 1, 1, 0), vec3(-1, 1, 0) },
		{ vec3(1,-1, 0), vec3(-1,-1, 0) }
	};*/
	//genBezierSurface("Assets/bezier.obj",16,16,k);
	//genCone("Assets/cone.obj", 8);
	//auto bezier = loadOBJ("Assets/bezier.obj");
	auto cone = loadOBJ("Assets/cone.obj");
	dm = make_shared<DrawMesh>(cone, "Assets/texture.png", prog);
	mesh = make_shared<ColliderEntity>(dm);
	mesh->transform.position = vec3(2.5f, 0, 0);
	//mesh->id = (void*)0xb;
	mesh->id = (void*)0xc1;
	mainState->addEntity(mesh);

	//genCylinder("Assets/cylinder.obj", 64);
	auto cylinder = loadOBJ("Assets/cylinder.obj");
	dm = make_shared<DrawMesh>(cylinder, "Assets/texture.png", prog);
	mesh = make_shared<ColliderEntity>(dm);
	mesh->id = (void*)0xc;
	mesh->transform.position = vec3(-2.5f, 0, 0);
	mainState->addEntity(mesh);
	mesh->rigidBody.floating(1);

	//genSphere("Assets/sphere.obj", 16);
	auto sphere = loadOBJ("Assets/sphere.obj");
	dm = make_shared<DrawMesh>(sphere, "Assets/texture.png", prog);
	mesh = make_shared<ColliderEntity>(dm);
	mesh->id = (void*)0xcc;
	mesh->transform.position = vec3(0, 2.5f, 0);
	mainState->addEntity(mesh);
	mesh->rigidBody.floating(1);

	//genCube("Assets/cube.obj");
	auto cube = loadOBJ("Assets/cube.obj");
	dm = make_shared<DrawMesh>(cube, "Assets/texture.png", prog);
	mesh = make_shared<ColliderEntity>(dm);
	mesh->id = (void*)0xc2fb;
	mesh->transform.position = vec3(0, -5.f, 0);
	mesh->transform.scale = vec3(64, 1.5f, 64);
	mesh->rigidBody.fixed(1);
	mesh->rigidBody.mass(10000);
	mainState->addEntity(mesh);

	dm = make_shared<DrawMesh>(cube, "Assets/texture.png", prog);
	mesh = make_shared<ColliderEntity>(dm);
	mesh->id = (void*)0xc2;
	mesh->transform.position = vec3(-2.5f, -2.5f, 0);
	mesh->rigidBody.floating(1);
	mainState->addEntity(mesh);
	me = mesh;

	camera = make_shared<Camera>(prog, window);
	camera->id = (void*)0xcab;
	camera->transform.position = vec3(0, 0, 1);
	camera->transform.rotate(0, PI, 0);
	mainState->addEntity(camera);

	if(DEBUG)
		DrawDebug::getInstance().camera(camera.get());
}

#include "CollisionManager.h"
void TriPlay::update(GLFWwindow* window, Mouse* m, double delta) {
	//if (!started) {
	//	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) started = true;
	//	return;
	//}

	const auto dt = (float)delta;

	Game::update(delta);
	CollisionManager::getInstance().update(dt);

	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
		//quit the game
		glfwTerminate();
		exit('q');
	}

	const auto speed = 5.f;

	bool shift = false, ctrl = false;
	if (glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		shift = true;
	if (glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		ctrl = true;

	if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) {
		if (shift)
			me->transform.position += vec3(0, 0, -speed * dt);
		else if (ctrl)
			me->transform.rotate(-2 * PI * dt, 0, 0);
		else
			me->transform.position += vec3(0, speed * dt, 0);
	}
	else if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
		if(shift)
			me->transform.position += vec3(0, 0, speed * dt);
		else if (ctrl)
			me->transform.rotate(2 * PI * dt, 0, 0); 
		else
			me->transform.position += vec3(0, -speed * dt, 0); 
	}
	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
		if (shift)
			me->transform.rotate(0, 2 * PI * dt, 0);
		else if (ctrl)
			me->transform.rotate(0, 0, 2 * PI * dt);
		else
			me->transform.position += vec3(speed * dt, 0, 0);
	}
	else if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
		if (shift)
			me->transform.rotate(0, -2 * PI * dt, 0);
		else if (ctrl)
			me->transform.rotate(0, 0, -2 * PI * dt);
		else
			me->transform.position += vec3(-speed * dt, 0, 0);
	}

	Camera::mayaCam(window, m, dt, camera.get());

	DrawDebug::getInstance().drawDebugVector(vec3(), vec3(1, 0, 0), vec3(1, 0, 0));
	DrawDebug::getInstance().drawDebugVector(vec3(), vec3(0, 1, 0), vec3(0, 0, 1));
	DrawDebug::getInstance().drawDebugVector(vec3(), vec3(0, 0, 1), vec3(0, 1, 0));
}