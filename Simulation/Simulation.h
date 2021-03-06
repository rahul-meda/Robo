
#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include "Body.h"
#include "Collider.h"
#include "Contact.h"
#include "PositionJoint.h"
#include "MouseJoint.h"
#include "PlaneConstraint.h"
#include "RevoluteJoint.h"

class Poly;

class Simulation
{
protected:
	// window dimensions
	int width;
	int height;

	// camera transformation
	float yaw;
	float pitch;

	// mouse co-ordinates
	float mouseX;
	float mouseY;

	// for panning the scene on corner scroll
	bool panLeft, panRight, panTop, panBot;

	// to check for first mouse callback
	bool firstMouseCB;

	// store the state of pressed keys
	bool keys[1024];

	// toggle drawing wire-frame mode
	bool debugDraw;

	bool pauseStep;
	bool advanceStep;

	std::vector<Body> bodies;
	std::vector<Collider*> colliders;
	std::vector<Manifold> manifolds;
	std::vector<PositionJoint> posJoints;
	std::vector<PlaneConstraint> planeConstraints;
	std::vector<RevoluteJoint> revJoints;

	MouseJoint mouseJoint;
	bool picked;

	Poly* boxModel;
	Model* cylinder;
	Model* sphere;
	Model* hemiSphere;

public:
	Simulation();

	~Simulation();

	virtual void OnInit(GLFWwindow* window);

	void OnWindowResize(GLFWwindow* window, int width, int height);

	void OnMouseMove(GLFWwindow* window, double x, double y);

	void OnMouseScroll(GLFWwindow* window, double dx, double dy);

	void OnMouseButton(GLFWwindow* window, int button, int action, int mods);

	virtual void OnKeyInput(GLFWwindow* window, int key, int code, int action, int mods);

	void Step(const float dt);

	virtual void Update();
};