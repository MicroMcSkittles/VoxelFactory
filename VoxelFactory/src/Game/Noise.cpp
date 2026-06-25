#include "Game/Noise.h"
#include <vector>
#include <glad/glad.h>

void NoiseGenerator::Init() {

}

Ref<Texture> NoiseGenerator::GenerateWhiteNoise(int width, int height, uint32_t seed) {
	std::vector<uint8_t> data;
	data.reserve(width * height);

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			uint32_t state = x + width * y;
			uint32_t hash = PCGHash(state, seed);
			float color = (float)hash / (float)std::numeric_limits<uint32_t>::max();
			data.push_back((uint8_t)(color * 255));
		}
	}

	return CreateRef<Texture>(data.data(), width, height, GL_R8, GL_RED);
}

Ref<Texture> NoiseGenerator::GeneratePerlinNoise(int width, int height, int frequency, const glm::vec2& offset, uint32_t seed)
{
	std::vector<uint8_t> data;
	data.reserve(width * height);

	int lattice_width = 2 * frequency;
	int lattice_height = ceil((float)(lattice_width * height) / (float)width);

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			glm::vec2 lattice_pos = {
				//(offset.x + ((float)x / 16.0f)) * (float)(lattice_width - 1),
				//(offset.y + ((float)y / 16.0f)) * (float)(lattice_height - 1)
				((float)x / (float)width) * (float)(lattice_width - 1),
				((float)y / (float)height) * (float)(lattice_height - 1)
			};

			glm::vec2 cell_min = { floor(lattice_pos.x), floor(lattice_pos.y) };
			glm::vec2 cell_max = cell_min + glm::vec2(1.0f);

			glm::vec2 top_left_gradient = RandGradient(cell_min, seed);
			glm::vec2 top_right_gradient = RandGradient(glm::vec2(cell_max.x, cell_min.y), seed);
			glm::vec2 bottom_left_gradient = RandGradient(glm::vec2(cell_min.x, cell_max.y), seed);;
			glm::vec2 bottom_right_gradient = RandGradient(cell_max, seed);

			float top_left_dot = glm::dot(top_left_gradient, lattice_pos - cell_min);
			float top_right_dot = glm::dot(top_right_gradient, lattice_pos - glm::vec2(cell_max.x, cell_min.y));
			float bottom_left_dot = glm::dot(bottom_left_gradient, lattice_pos - glm::vec2(cell_min.x, cell_max.y));
			float bottom_right_dot = glm::dot(bottom_right_gradient, lattice_pos - cell_max);

			glm::vec2 interp_weight = lattice_pos - cell_min;

			float upper = CubicInterp(top_left_dot, top_right_dot, interp_weight.x);
			float lower = CubicInterp(bottom_left_dot, bottom_right_dot, interp_weight.x);
			float value = CubicInterp(upper, lower, interp_weight.y);

			value = ((value + 1.0f) / 2.0f) * 255.0f;
			value = std::max(std::min(value, 255.0f), 0.0f);
			data.push_back((uint8_t)value);
		}
	}

	return CreateRef<Texture>(data.data(), width, height, GL_R8, GL_RED);
}

float NoiseGenerator::SamplePerlinNoise(const glm::vec2& position, const glm::vec2& offset, int frequency, uint32_t seed)
{
	float lattice_size = 1.0f / frequency;
	glm::vec2 lattice_pos = {
		(offset.x + (position.x / 16.0f)) * (float)(lattice_size - 1),
		(offset.y + (position.y / 16.0f)) * (float)(lattice_size - 1)
	};

	glm::vec2 cell_min = { floor(lattice_pos.x), floor(lattice_pos.y) };
	glm::vec2 cell_max = cell_min + glm::vec2(1.0f);

	glm::vec2 top_left_gradient = RandGradient(cell_min, seed);
	glm::vec2 top_right_gradient = RandGradient(glm::vec2(cell_max.x, cell_min.y), seed);
	glm::vec2 bottom_left_gradient = RandGradient(glm::vec2(cell_min.x, cell_max.y), seed);;
	glm::vec2 bottom_right_gradient = RandGradient(cell_max, seed);

	float top_left_dot = glm::dot(top_left_gradient, lattice_pos - cell_min);
	float top_right_dot = glm::dot(top_right_gradient, lattice_pos - glm::vec2(cell_max.x, cell_min.y));
	float bottom_left_dot = glm::dot(bottom_left_gradient, lattice_pos - glm::vec2(cell_min.x, cell_max.y));
	float bottom_right_dot = glm::dot(bottom_right_gradient, lattice_pos - cell_max);

	glm::vec2 interp_weight = lattice_pos - cell_min;

	float upper = CubicInterp(top_left_dot, top_right_dot, interp_weight.x);
	float lower = CubicInterp(bottom_left_dot, bottom_right_dot, interp_weight.x);
	float value = CubicInterp(upper, lower, interp_weight.y);

	return value;
}

float NoiseGenerator::CubicInterp(float v1, float v2, float weight) {
	return (v2 - v1) * (3.0f - weight * 2.0f) * weight * weight + v1;
}
glm::vec2 NoiseGenerator::RandGradient(const glm::vec2& position, uint32_t seed) {
	uint32_t state = (position.x + position.y) * position.y;
	state = PCGHash(state, seed);
	state ^= (uint32_t)position.x;
	return RandDirection(state, seed);
}
glm::vec2 NoiseGenerator::RandDirection(uint32_t& state, uint32_t seed) {
	return glm::normalize(glm::vec2(
		NormalizedRand(state, seed),
		NormalizedRand(state, seed)
	));
}

uint32_t NoiseGenerator::PCGHash(uint32_t& state, uint32_t seed) {
	state = state * 747796405u + 2891336453u + seed;
	uint32_t word = ((state >> ((state >> (28u * (seed + 1))) + 4u)) ^ state) * 277803737u;
	return (word >> 22u) ^ word;
}
float NoiseGenerator::NormalizedRand(uint32_t& state, uint32_t seed) {
	return ((float)PCGHash(state, seed) / (float)std::numeric_limits<uint32_t>::max()) * 2.0f - 1.0f;
}
