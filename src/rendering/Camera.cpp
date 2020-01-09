#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>

namespace vmc
{
	Camera::Camera() :
		position(0, 0, 0),
		pitch(0),
		yaw(0)
	{
	}

	void Camera::setPosition(const glm::vec3& value)
	{
		position = value;
	}

	void Camera::addPitch(float delta)
	{
		static float maxPitch = glm::pi<float>() / 2.0f - 0.00001f;
		static float minPitch = -maxPitch;

		pitch += delta;

		if (pitch > maxPitch) {
			pitch = maxPitch;
		}

		if (pitch < minPitch) {
			pitch = minPitch;
		}
	}

	void Camera::addYaw(float delta)
	{
		yaw += delta;
	}

	void Camera::moveForward(float delta)
	{
		auto direction = getLookDirection();
		direction.y = 0;
		position += direction * delta;
	}

	void Camera::moveSide(float delta)
	{
		auto direction = getLookDirection();
		direction.y = 0;
		direction = glm::normalize(glm::cross(direction, glm::vec3(0, 1, 0)));
		direction.y = 0;
		position += direction * delta;
	}

	glm::vec3 Camera::getPosition() const
	{
		return position;
	}

	glm::vec3 Camera::getLookDirection()
	{
		glm::vec3 lookDirection(0, 0, -1);
		glm::vec3 pitchAxis(1, 0, 0);

		auto yawRotation = glm::rotate(glm::identity<glm::mat4>(), yaw, glm::vec3(0, 1, 0));
		lookDirection = yawRotation * glm::vec4(lookDirection, 1.0f);
		pitchAxis = yawRotation * glm::vec4(pitchAxis, 1.0f);

		auto pitchRotation = glm::rotate(glm::identity<glm::mat4>(), pitch, pitchAxis);
		lookDirection = pitchRotation * glm::vec4(lookDirection, 1.0f);

		return glm::normalize(lookDirection);
	}

	glm::mat4 Camera::getViewMatrix()
	{
		return glm::lookAt(position, position + getLookDirection(), glm::vec3(0, 1, 0));
	}
}