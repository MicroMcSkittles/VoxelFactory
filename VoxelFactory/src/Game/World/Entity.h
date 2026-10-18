#pragma once
#include <functional>
#include <glm/glm.hpp>

struct Transform {
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;

	glm::mat4 GetMatrix(const glm::mat4& parent = glm::mat4(1.0f));

	Transform() : position(0.0f), rotation(0.0f), scale(1.0f) { }
	Transform(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale) 
		: position(position), rotation(rotation), scale(scale) { }

	static Transform LinearInterp(const Transform& t1, const Transform& t2, float time);
	static Transform CubicInterp(const Transform& t1, const Transform& t2, float time);
};

// TODO: actual entities