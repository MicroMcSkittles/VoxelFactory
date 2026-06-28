#pragma once
#include <glm/glm.hpp>

struct Frustum {
	float aspect_ratio;
	float fov;
	float near;
	float far;
};

struct Camera {
	Frustum frustum;
	glm::vec3 position;
	glm::vec3 direction; // A point 1 unit from m_Position
	glm::vec3 eular; // pitch, yaw, roll

	glm::mat4 view;
	glm::mat4 projection;
	glm::mat4 view_projection;

	// yaw and pitch are in radians
	static glm::vec3 EulerDirection(float pitch, float yaw);

	void UpdateView();
	void UpdateProjection();

	Camera(const Frustum& frustum, const glm::vec3& position, const glm::vec3& direction);
};

struct ViewBox {
	float width;
	float height;
	float near;
	float far;
	float scale;
};
struct OrthographicCamera {

	ViewBox view_box;
	glm::vec3 position;

	glm::mat4 view;
	glm::mat4 projection;
	glm::mat4 view_projection;

	void UpdateView();
	void UpdateProjection();

	OrthographicCamera(const ViewBox& view_box, const glm::vec3& position);
};