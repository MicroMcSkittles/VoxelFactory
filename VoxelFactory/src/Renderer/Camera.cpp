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

OrthographicCamera::OrthographicCamera(const ViewBox& view_box, const glm::vec3& position)
	: view_box(view_box), position(position) 
{
	float aspect_ratio = view_box.width / view_box.height;
	projection = glm::ortho(-aspect_ratio, aspect_ratio, -1.0f, 1.0f, view_box.near, view_box.far);
	view = glm::translate(glm::mat4(1.0f), -position);
	view_projection = projection * view;
}
void OrthographicCamera::UpdateView() {
	view = glm::translate(glm::mat4(1.0f), -position);
	view_projection = projection * view;
}
void OrthographicCamera::UpdateProjection() {
	float aspect_ratio = view_box.width / view_box.height;
	projection = glm::ortho(-aspect_ratio, aspect_ratio, -1.0f, 1.0f, view_box.near, view_box.far);
	view_projection = projection * view;
}