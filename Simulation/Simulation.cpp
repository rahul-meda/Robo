
#include "Simulation.h"
#include "Poly.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include "Camera.h"
#include <memory>
#include "NarrowPhase.h"
#include "Mesh.h"
#include "ObjParser.h"

#define MOUSE_SENSITIVITY 0.1

Simulation::Simulation()
	:debugDraw(false), firstMouseCB(false), pauseStep(true), advanceStep(false)
{
	panLeft = panRight = panBot = panTop = false;
}

void Simulation::OnInit(GLFWwindow* window)
{
	glfwGetWindowSize(window, &width, &height);

	mouseX = width / 2.0f;
	mouseY = height / 2.0f;

	glClearColor(0.9, 0.9, 0.9, 0);

	// draw the pixel only if the object is closer to the viewer
	glEnable(GL_DEPTH_TEST); // enables depth-testing
	glDepthFunc(GL_LESS);    // interpret smaller values as closer

	Camera::GetInstance().SetProjection(45.0f, (float)width / (float)height);
	Camera::GetInstance().SetPosition(glm::vec3(0, 1, 0));

	HMesh mesh;
	ParseObj("resources/box.obj", mesh);
	std::vector<glm::vec3> vertices;
	std::vector<int> indices;

	for (auto vert : mesh.vertices)
	{
		vertices.push_back(vert->position);
	}
	mesh.GetTriangleIndices(indices);
	boxModel = new Poly(vertices, indices);
	boxModel->SetColor(glm::vec3(1.0, 0, 0));

	// reserving space for global data
	// otherwise any pointers or references to container elements will get invalidated on using push_back
	// because push_back reallocates memory, and old memory locations are invalidated
	bodies.reserve(5000);
	colliders.reserve(5000);
	contacts.reserve(5000);
}

void Simulation::OnWindowResize(GLFWwindow* window, int width, int height)
{
	this->width = width;
	this->height = height;
	glViewport(0, 0, width, height);

	Camera::GetInstance().SetProjection(45.0f, (float)width / height);
}

void Simulation::OnMouseMove(GLFWwindow* window, double x, double y)
{
	if (firstMouseCB)
	{
		mouseX = x;
		mouseY = y;
		firstMouseCB = false;
	}

	float dx = mouseX - x;
	float dy = mouseY - y;

	mouseX = x;
	mouseY = y;

	dx *= MOUSE_SENSITIVITY;
	dy *= MOUSE_SENSITIVITY;

	yaw += dx;
	pitch += dy;

	if (mouseX < 20) panLeft = true;
	else panLeft = false;
	if (mouseX > width - 20) panRight = true;
	else panRight = false;
	if (mouseY < 20) panBot = true;
	else panBot = false;
	if (mouseY > height - 20) panTop = true;
	else panTop = false;

	if (pitch > 89.0f) pitch = 89.0f;
	if (pitch < -89.0f) pitch = -89.0f;

	Camera::GetInstance().Rotate(yaw, pitch, 0);
}

void Simulation::OnMouseScroll(GLFWwindow* window, double dx, double dy)
{
}

void Simulation::OnMouseButton(GLFWwindow* window, int button, int action, int mods)
{

}

void Simulation::OnKeyInput(GLFWwindow* window, int key, int code, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_RIGHT_ALT && action == GLFW_PRESS)
		debugDraw = !debugDraw;

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			keys[key] = true;
		else if (action == GLFW_RELEASE)
			keys[key] = false;
	}

	if (keys[GLFW_KEY_W])
		Camera::GetInstance().Move(Camera::GetInstance().GetCamZ());
	if (keys[GLFW_KEY_S])
		Camera::GetInstance().Move(-Camera::GetInstance().GetCamZ());
	if (keys[GLFW_KEY_A])
		Camera::GetInstance().Move(-Camera::GetInstance().GetCamX());
	if (keys[GLFW_KEY_D])
		Camera::GetInstance().Move(Camera::GetInstance().GetCamX());
	if (keys[GLFW_KEY_Q])
		Camera::GetInstance().Move(Camera::GetInstance().GetCamY());
	if (keys[GLFW_KEY_Z])
		Camera::GetInstance().Move(-Camera::GetInstance().GetCamY());

	if (keys[GLFW_KEY_P])
		pauseStep = !pauseStep;
	if (keys[GLFW_KEY_N])
		advanceStep = true;
}

void Simulation::Step(const float dt)
{
	for (Body& body : bodies)
	{
		body.Update(dt);
	}

	contacts.clear();

	// To Do : Broadphase

	for (int iA = 0; iA < colliders.size(); iA++)
	{
		for (int iB = iA + 1; iB < colliders.size(); iB++)
		{
			DetectCollision(contacts, colliders[iA], colliders[iB]);
		}
	}
}

void Simulation::Update()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Physics Update
	const static float dt = 1.0f / 60.0f;

	if (!pauseStep)
		Step(dt);
	else
	{
		if (advanceStep)
		{
			Step(dt);
			advanceStep = false;
		}
	}

	// Graphics update
	glm::mat4 T(1), R(1), S(1), M(1), MVP(1);
	glm::mat4 V = Camera::GetInstance().GetViewMatrix();
	glm::mat4 P = Camera::GetInstance().GetProjectionMatrix();

	for (auto collider : colliders)
	{
		collider->Render();
	}

	for (auto contact : contacts)
	{
		T = glm::translate(contact.position);
		S = glm::scale(glm::vec3(0.1f));
		M = T * S;
		MVP = P * V * M;
		boxModel->SetMVP(MVP);
		boxModel->Render();
	}

	// Debug Draw
	if (debugDraw)
	{
		// wire frame mode
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	if (panLeft)
	{
		yaw += MOUSE_SENSITIVITY;
		Camera::GetInstance().Rotate(yaw, pitch, 0);
	}
	if (panRight)
	{
		yaw -= MOUSE_SENSITIVITY;
		Camera::GetInstance().Rotate(yaw, pitch, 0);
	}
	if (panTop)
	{
		pitch += MOUSE_SENSITIVITY;
		Camera::GetInstance().Rotate(yaw, pitch, 0);
	}
	if (panBot)
	{
		pitch -= MOUSE_SENSITIVITY;
		Camera::GetInstance().Rotate(yaw, pitch, 0);
	}
}

Simulation::~Simulation()
{}