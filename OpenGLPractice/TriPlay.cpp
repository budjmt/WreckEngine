#include "TriPlay.h"

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
	Mesh* m = loadOBJ("Assets/basic.obj", "Assets/texture.png", prog);
	shapes.push_back(m);
	mesh = new Entity(m);
	mesh->transform.rotAxis = glm::vec3(0, 1, 0);
	mesh->transform.position.z = 0;
	entities.push_back(mesh);

	camera = new Camera(prog);
	camera->window = window;
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
	delete mesh;
}

void TriPlay::update(GLFWwindow* window, Mouse* m, double prevFrame, double dt) {
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
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	if (m->down) {
		//mesh->transform.rotation += (float)(2 * M_PI * dt);
		if (m->button == GLFW_MOUSE_BUTTON_LEFT) {
			camera->turn((float)(m->x - m->prevx), (float)(m->y - m->prevy));
		}
		else if (m->button == GLFW_MOUSE_BUTTON_RIGHT) {
			camera->zoom += (float)((m->y - m->prevy) / height);
		}
		else if (m->button == GLFW_MOUSE_BUTTON_MIDDLE) {
			camera->transform.position += glm::vec3(m->x - m->prevx, m->y - m->prevy, 0);
		}
	}

	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
		//quit the game
		glfwTerminate();
		exit('q');
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		camera->transform.position += camera->getForward() * 5.f * (float)dt;
	}
	else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		camera->transform.position += camera->getForward() * -5.f * (float)dt;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		camera->transform.position += camera->getRight() * 5.f * (float)dt;
	}
	else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		camera->transform.position += camera->getRight() * -5.f * (float)dt;
	}
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