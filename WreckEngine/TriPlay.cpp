#include "TriPlay.h"

#include "TextEntity.h"

#include <iostream>

namespace {
	void menu_update(LogicEntity* e, double dt) {
		if (Window::getKey(GLFW_KEY_SPACE) == GLFW_PRESS) {
			EventTrigger(e).sendEvent(EventHandler::get("menu_state"), Event::get("start_game"));
		}
	}
}

TriPlay::TriPlay(GLprogram prog) : Game(prog)
{
	program = prog;

	auto menuState = make_shared<State>("menu");
	auto mainState = make_shared<State>("main");
	auto mainsp = mainState.get();

	auto start_game_event = Event::add("start_game");
	menuState->handler_func = [this, mainsp, start_game_event](Event e) {
		if (e.id == start_game_event) {
			currState = mainsp;
			glClearColor(0, 0, 0, 1);
		}
	};
	addState(menuState);
	menuState->addEntity(make_shared<LogicEntity>(menu_update));
	
	auto menuText = make_shared<TextEntity>("Press space to begin.", "QuartzMS.ttf", Text::Justify::MIDDLE, Text::Justify::MIDDLE, 48);
	menuText->transform.position = vec3(Window::width * 0.5f, Window::height * 0.5f, 0);
	menuText->transform.scale = vec3(0.5f, 1, 1);
	menuState->addEntity(menuText);
	//Text::active = false;

	glClearColor(0, 0.5f, 0.2f, 1);

	mainState->handler_func = [](Event e) {
		//nothing right now
	};
	addState(mainState);

	auto m = loadOBJ("Assets/basic.obj");
	m->translateTo(vec3());
	auto mesh = make_shared<ColliderEntity>(make_shared<DrawMesh>(m, "Assets/texture.png", prog));
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
	mesh = make_shared<ColliderEntity>(make_shared<DrawMesh>(cone, "Assets/texture.png", prog));
	mesh->transform.position = vec3(2.5f, 0, 0);
	//mesh->id = (void*)0xb;
	mesh->id = (void*)0xc1;
	mainState->addEntity(mesh);

	//genCylinder("Assets/cylinder.obj", 64);
	auto cylinder = loadOBJ("Assets/cylinder.obj");
	mesh = make_shared<ColliderEntity>(make_shared<DrawMesh>(cylinder, "Assets/texture.png", prog));
	mesh->id = (void*)0xc;
	mesh->transform.position = vec3(-2.5f, 0, 0);
	mainState->addEntity(mesh);
	mesh->rigidBody.floating(1);

	//genSphere("Assets/sphere.obj", 16);
	auto sphere = loadOBJ("Assets/sphere.obj");
	mesh = make_shared<ColliderEntity>(make_shared<DrawMesh>(sphere, "Assets/texture.png", prog));
	mesh->id = (void*)0xcc;
	mesh->transform.position = vec3(0, 2.5f, 0);
	mainState->addEntity(mesh);
	mesh->rigidBody.floating(1);

	//genCube("Assets/cube.obj");
	auto cube = loadOBJ("Assets/cube.obj");
	mesh = make_shared<ColliderEntity>(make_shared<DrawMesh>(cube, "Assets/texture.png", prog));
	mesh->id = (void*)0xc2fb;
	mesh->transform.position = vec3(0, -5.f, 0);
	mesh->transform.scale = vec3(64, 1.5f, 64);
	mesh->rigidBody.fixed(1);
	mesh->rigidBody.mass(10000);
	mainState->addEntity(mesh);

	mesh = make_shared<ColliderEntity>(make_shared<DrawMesh>(cube, "Assets/butt.png", prog));
	mesh->id = (void*)0xc2;
	mesh->transform.position = vec3(-2.5f, -2.5f, 0);
	mesh->rigidBody.floating(1);
	mainState->addEntity(mesh);
	me = mesh;

	auto camera = make_shared<Camera>(prog);
	camera->id = (void*)0xcab;
	camera->transform.position = vec3(0, 0, 1);
	camera->transform.rotate(0, PI, 0);
	mainState->addEntity(camera);

	if(DEBUG) DrawDebug::getInstance().camera(camera.get());
}

#include "CollisionManager.h"
void TriPlay::update(double delta) {
	const auto dt = (float)delta;

	Game::update(delta);
	CollisionManager::getInstance().update(dt);

	//quit the game
	if (Window::getKey(GLFW_KEY_Q) == GLFW_PRESS) exit('q');

	constexpr auto speed = 5.f;

	bool shift = Window::getKey(GLFW_KEY_RIGHT_SHIFT)   == GLFW_PRESS || Window::getKey(GLFW_KEY_LEFT_SHIFT)   == GLFW_PRESS;
	bool ctrl  = Window::getKey(GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS || Window::getKey(GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS;

	if (Window::getKey(GLFW_KEY_I) == GLFW_PRESS) {
		if (shift)
			me->transform.position += vec3(0, 0, -speed * dt);
		else if (ctrl)
			me->transform.rotate(-2 * PI * dt, 0, 0);
		else
			me->transform.position += vec3(0, speed * dt, 0);
	}
	else if (Window::getKey(GLFW_KEY_K) == GLFW_PRESS) {
		if(shift)
			me->transform.position += vec3(0, 0, speed * dt);
		else if (ctrl)
			me->transform.rotate(2 * PI * dt, 0, 0); 
		else
			me->transform.position += vec3(0, -speed * dt, 0); 
	}
	if (Window::getKey(GLFW_KEY_L) == GLFW_PRESS) {
		if (shift)
			me->transform.rotate(0, 2 * PI * dt, 0);
		else if (ctrl)
			me->transform.rotate(0, 0, 2 * PI * dt);
		else
			me->transform.position += vec3(speed * dt, 0, 0);
	}
	else if (Window::getKey(GLFW_KEY_J) == GLFW_PRESS) {
		if (shift)
			me->transform.rotate(0, -2 * PI * dt, 0);
		else if (ctrl)
			me->transform.rotate(0, 0, -2 * PI * dt);
		else
			me->transform.position += vec3(-speed * dt, 0, 0);
	}

	Camera::mayaCam(Camera::main, dt);
	program.use();
	Camera::main->updateCamMat(Camera::main->cameraMatrix);

	DrawDebug::getInstance().drawDebugVector(vec3(), vec3(1, 0, 0), vec3(1, 0, 0));
	DrawDebug::getInstance().drawDebugVector(vec3(), vec3(0, 1, 0), vec3(0, 0, 1));
	DrawDebug::getInstance().drawDebugVector(vec3(), vec3(0, 0, 1), vec3(0, 1, 0));
}

void TriPlay::draw() {
	program.use();
	Game::draw();
	Text::render();
}