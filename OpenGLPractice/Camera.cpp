#include "Camera.h"

Camera::Camera(GLuint shaderProg, GLFWwindow* w)
{
	window = w;

	updateProjection();

	cameraMatrix = glGetUniformLocation(shaderProg, "cameraMatrix");
}

Camera::~Camera()
{
}

void Camera::update(double dt) {
	glm::mat4 view = glm::lookAt(transform.position,getLookAt(),getUp());
	//update projection
	//updateProjection();
	glm::mat4 tmp = projection * view;
	glUniformMatrix4fv(cameraMatrix,1,GL_FALSE,&(tmp[0][0]));
}

void Camera::draw() {
	//does NOTHING because it's a CAMERA
	//or maybe there's debug here
	//who knows
}

void Camera::turn(float dx, float dy) {
	transform.rotate(dy, dx, 0);
}

glm::vec3 Camera::getLookAt() {
	return transform.position + getForward();
}

void Camera::updateProjection() {
	int width, height;
	glfwGetWindowSize(window, &width, &height);

	float znear = 0.01f;
	float zfar = 1000.f;
	projection = glm::perspective(CAM_FOV, width * 1.f / height, znear, zfar);
}

glm::vec3 Camera::getForward() {
	return transform.getForward();
}

glm::vec3 Camera::getUp() {
	return transform.getUp();
}

glm::vec3 Camera::getRight() {
	return transform.getRight();
}

void Camera::mayaCam(GLFWwindow* window, Mouse* m, double dt, Camera* camera) {
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	if (m->down) {
		if (m->button == GLFW_MOUSE_BUTTON_LEFT) {
			float rot = (float)(glm::pi<float>() / 2 / dt);
			float dx = (float)(m->x - m->prevx) / width * rot;
			float dy = (float)(m->y - m->prevy) / height * rot;
			glm::vec3 look = camera->getLookAt();
			camera->turn(dx, dy);
			camera->transform.position = look - camera->getForward();
		}
		else if (m->button == GLFW_MOUSE_BUTTON_RIGHT) {
			float avg = ((m->y - m->prevy) + (m->x - m->prevx)) / 2;
			camera->transform.position += avg * camera->getForward();
		}
		else if (m->button == GLFW_MOUSE_BUTTON_MIDDLE) {
			camera->transform.position += camera->getRight() * (float)(m->x - m->prevx);
			camera->transform.position += camera->getUp() * (float)(m->y - m->prevy);
		}
		//I have this commented out on purpose. I don't want it
		//glfwSetCursorPos(window, width / 2, height / 2);
		//std::cout << "Position: " << camera->transform.position.x << "," << camera->transform.position.y << "," << camera->transform.position.z << std::endl << "Pitch: " << camera->pitch << std::endl << "Yaw: " << camera->yaw << std::endl;
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