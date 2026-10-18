#include "Game/World/Entity.h"
#include <glm/gtc/matrix_transform.hpp>

glm::mat4 Transform::GetMatrix(const glm::mat4& parent) {
	glm::mat4 model = parent;
	model = glm::translate(model, position);
	model = glm::rotate(model, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, scale);
	return model;
}

Transform Transform::LinearInterp(const Transform& t1, const Transform& t2, float time) {
	Transform result;
	result.position = (t2.position - t1.position) * time + t1.position;
	result.rotation = (t2.rotation - t1.rotation) * time + t1.rotation;
	result.scale = (t2.scale - t1.scale) * time + t1.scale;
	return result;
}
Transform Transform::CubicInterp(const Transform& t1, const Transform& t2, float time) {
	Transform result;
	result.position = (t2.position - t1.position) * (3.0f - time * 2.0f) * time * time + t1.position;
	result.rotation = (t2.rotation - t1.rotation) * (3.0f - time * 2.0f) * time * time + t1.rotation;
	result.scale = (t2.scale - t1.scale) * (3.0f - time * 2.0f) * time * time + t1.scale;
	return result;
}
