
#define CAM_SPEED 25.0f
#define DAMPING 0.97f
#define SPEED_EPSILON 0.79f

#include "Camera.h"
#include <glm/gtx/euler_angles.hpp>

Camera::Camera()
	:position(0.0), R(1.0), V(1.0), P(1.0)
{
}

Camera& Camera::GetInstance()
{
	static Camera instance;
	return instance;
}

void Camera::SetProjection(const float fov, const float aspectRatio, const float n, const float f)
{
	this->fov = fov;
	this->aspectRatio = aspectRatio;
	zNear = n;
	zFar = f;

	P = glm::perspective(fov, aspectRatio, n, f);
}

const glm::mat4 Camera::GetViewMatrix() const
{
	return V;
}

const glm::mat4 Camera::GetProjectionMatrix() const
{
	return P;
}

const glm::mat4 Camera::GetVPMatrix() const
{
	return P*V;
}

void Camera::SetPosition(const glm::vec3& pos)
{
	position = pos;
}

const glm::vec3 Camera::GetPosition() const
{
	return position;
}

void Camera::Move(const glm::vec3& dir)
{
	const static float dt = 1.0f / 60.0f;

	position += CAM_SPEED*dt*dir*DAMPING;

	Update();
}

const glm::vec3 Camera::GetCamX() const
{
	return glm::normalize(right);
}

const glm::vec3 Camera::GetCamY() const
{
	return glm::normalize(up);
}

const glm::vec3 Camera::GetCamZ() const
{
	return glm::normalize(look);
}

void Camera::SetFOV(const float fov)
{
	this->fov = fov;
	P = glm::perspective(fov, aspectRatio, zNear, zFar);
}

const float Camera::GetFOV() const
{
	return fov;
}

const float Camera::GetAspectRatio() const
{
	return aspectRatio;
}

void Camera::Rotate(const float yaw, const float pitch, const float roll)
{
	float y = glm::radians(yaw);
	float p = glm::radians(pitch);
	float r = glm::radians(roll);

	R = glm::yawPitchRoll(y, p, r);

	Update();
}

void Camera::Update(glm::vec3 t)
{
	// camera z-axis
	look = glm::normalize(glm::vec3(R*glm::vec4(0, 0, -1, 0)));

	// camera y-axis
	up = glm::normalize(glm::vec3(R*glm::vec4(0, 1, 0, 0)));

	// camera x-axis
	right = glm::normalize(glm::cross(look, up));

	// target direction
	glm::vec3 target = position + look;
	//target = glm::normalize(target);

	V = glm::lookAt(position, target, up);
}