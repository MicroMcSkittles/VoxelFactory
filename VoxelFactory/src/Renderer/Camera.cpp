#include "Renderer/Camera.h"
#include <glm/gtc/matrix_transform.hpp>

glm::vec3 Camera::EulerDirection(float pitch, float yaw) {
	glm::vec3 direction;
	direction.x = cos(yaw) * cos(pitch);
	direction.y = sin(pitch);
	direction.z = sin(yaw) * cos(pitch);
	return glm::normalize(direction);
}

Camera::Camera(const Frustum& frustum, const glm::vec3& position, const glm::vec3& direction)
	:frustum(frustum), position(position), direction(direction), eular(0.0f)
{
	projection = glm::perspective(frustum.fov, frustum.aspect_ratio, frustum.near, frustum.far);
	view = glm::lookAt(position, position + direction, glm::vec3(0.0f, 1.0f, 0.0f));
	view_projection = projection * view;
}

void Camera::UpdateView() {
	view = glm::lookAt(position, position + direction, glm::vec3(0.0f, 1.0f, 0.0f));
	view_projection = projection * view;
}
void Camera::UpdateProjection() {
	projection = glm::perspective(frustum.fov, frustum.aspect_ratio, frustum.near, frustum.far);
	view_projection = projection * view;
}