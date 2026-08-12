#include "Game/Noise.h"
#include <vector>
#include <glad/glad.h>

Ref<Texture> NoiseGenerator::GenerateWhiteNoise(int width, int height, uint32_t seed) {
	std::vector<uint8_t> data;
	data.reserve(width * height);

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			//uint32_t state = x + width * y;
			//uint32_t hash = PCGHash(state, seed);
			//float color = (float)hash / (float)std::numeric_limits<uint32_t>::max();
			//data.push_back((uint8_t)(color * 255));
		}
	}

	return CreateRef<Texture>(data.data(), width, height, GL_R8, GL_RED);
}
Ref<Texture> NoiseGenerator::GeneratePerlinNoise(int width, int height, int frequency, const glm::vec2& offset, uint32_t seed)
{
	std::vector<uint8_t> data;
	data.reserve(width * height);

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			float value = SampleFractalPerlinNoise2D({ (float)x / (float)width,(float)y / (float)height }, 12, seed);
			uint8_t color = (uint8_t)(((value + 1.0f) * 0.5f) * 255);
			data.push_back(color);
		}
	}

	return CreateRef<Texture>(data.data(), width, height, GL_R8, GL_RED);
}

float NoiseGenerator::SampleWhiteNoise(const glm::vec2& position, uint32_t seed) {

	int n = position.x * 3 + position.y * 113;

	n = (n << 13) ^ n;
	n = n * (n * n * 15731 + 789221) + 1376312589;
	return -1.0 + 2.0 * float(n & 0x0fffffff) / float(0x0fffffff);
}

float NoiseGenerator::SamplePerlinNoise2D(const glm::vec2& position, uint32_t seed) {
	glm::ivec2 grid_min = { floor(position.x), floor(position.y) };
	glm::ivec2 grid_max = grid_min + 1;

	glm::vec2 weights = {
		position.x - (float)grid_min.x,
		position.y - (float)grid_min.y
	};

	// Interpolate top 2 corners
	float corner_0 = DotGridGradient2D(grid_min, position, seed);
	float corner_1 = DotGridGradient2D({ grid_max.x, grid_min.y }, position, seed);
	float top = CubicInterp(corner_0, corner_1, weights.x);

	// Interpolate bottom 2 corners
	corner_0 = DotGridGradient2D({ grid_min.x, grid_max.y }, position, seed);
	corner_1 = DotGridGradient2D(grid_max, position, seed);
	float bottom = CubicInterp(corner_0, corner_1, weights.x);

	return CubicInterp(top, bottom, weights.y);
}

float NoiseGenerator::SampleFractalPerlinNoise2D(const glm::vec2& position, int octave_count, uint32_t seed) {
	float value = 0.0f;

	float frequency = 1.0f;
	float amplitude = 1.0f;
	for (int i = 0; i < octave_count; i++) {
		value += SamplePerlinNoise2D({ position.x * frequency, position.y * frequency }, seed) * amplitude;
		frequency *= 2;
		amplitude *= 0.5f;
	}

	if (value > 1.0f) value = 1.0f;
	else if (value < -1.0f) value = -1.0f;

	return value;
}

float NoiseGenerator::SamplePerlinNoise3D(const glm::vec3& position, uint32_t seed) {
	glm::ivec3 grid_min = { floor(position.x), floor(position.y), floor(position.z) };
	glm::ivec3 grid_max = grid_min + 1;

	glm::vec3 weights = {
		position.x - (float)grid_min.x,
		position.y - (float)grid_min.y,
		position.z - (float)grid_min.z
	};

	// Interpolate top top 2 corners
	float corner_0 = DotGridGradient3D({ grid_min.x, grid_max.y, grid_min.z }, position, seed);
	float corner_1 = DotGridGradient3D({ grid_max.x, grid_max.y, grid_min.z }, position, seed);
	float top_0 = CubicInterp(corner_0, corner_1, weights.x);

	// Interpolate top bottom 2 corners
	corner_0 = DotGridGradient3D({ grid_min.x, grid_max.y, grid_max.z }, position, seed);
	corner_1 = DotGridGradient3D({ grid_max.x, grid_max.y, grid_max.z }, position, seed);
	float bottom_0 = CubicInterp(corner_0, corner_1, weights.x);

	float top = CubicInterp(top_0, bottom_0, weights.z);

	// Interpolate bottom top 2 corners
	corner_0 = DotGridGradient3D({ grid_min.x, grid_min.y, grid_min.z }, position, seed);
	corner_1 = DotGridGradient3D({ grid_max.x, grid_min.y, grid_min.z }, position, seed);
	top_0 = CubicInterp(corner_0, corner_1, weights.x);

	// Interpolate bottom bottom 2 corners
	corner_0 = DotGridGradient3D({ grid_min.x, grid_min.y, grid_max.z }, position, seed);
	corner_1 = DotGridGradient3D({ grid_max.x, grid_min.y, grid_max.z }, position, seed);
	bottom_0 = CubicInterp(corner_0, corner_1, weights.x);

	float bottom = CubicInterp(top_0, bottom_0, weights.z);

	return CubicInterp(bottom, top, weights.y);
}
float NoiseGenerator::SampleFractalPerlinNoise3D(const glm::vec3& position, int octave_count, uint32_t seed) {
	float value = 0.0f;

	float frequency = 1.0f;
	float amplitude = 1.0f;
	for (int i = 0; i < octave_count; i++) {
		value += SamplePerlinNoise3D(position * frequency, seed) * amplitude;
		frequency *= 2;
		amplitude *= 0.5f;
	}

	if (value > 1.0f) value = 1.0f;
	else if (value < -1.0f) value = -1.0f;

	return value;
}

float NoiseGenerator::CubicInterp(float v1, float v2, float weight) {
	return (v2 - v1) * (3.0f - weight * 2.0f) * weight * weight + v1;
}
float NoiseGenerator::DotGridGradient2D(const glm::ivec2& gradient_position, const glm::vec2& position, uint32_t seed) {
	glm::vec2 gradient = RandomGradient2D(gradient_position, seed);
	glm::vec2 distance = position - glm::vec2((float)gradient_position.x, (float)gradient_position.y);
	return glm::dot(distance, gradient);
}
glm::vec2 NoiseGenerator::RandomGradient2D(const glm::ivec2& position, uint32_t seed) {
	uint32_t state = (seed << PCGHash(position.y)) ^ (seed >> PCGHash(position.x));
	float theta = RandomFloatRange(state, 0.0f, PI2);
	return {
		cos(theta),
		sin(theta)
	};
}

float NoiseGenerator::DotGridGradient3D(const glm::ivec3& gradient_position, const glm::vec3& position, uint32_t seed) {
	glm::vec3 gradient = RandomGradient3D(gradient_position, seed);
	glm::vec3 distance = position - glm::vec3((float)gradient_position.x, (float)gradient_position.y, (float)gradient_position.z);
	return glm::dot(distance, gradient);
}

glm::vec3 NoiseGenerator::RandomGradient3D(const glm::ivec3& position, uint32_t seed) {
	uint32_t state = (seed << PCGHash(position.y)) ^ (seed >> PCGHash(position.x)) ^ (seed << PCGHash(position.z));
	float theta = RandomFloatRange(state, 0.0f, PI2);
	float phi = RandomFloatRange(state, 0.0f, PI2);

	return {
		sin(theta) * cos(phi),
		sin(theta) * sin(phi),
		cos(theta)
	};
}

uint32_t NoiseGenerator::State(const glm::vec3& input, uint32_t seed) {
	return (seed << (int)input.x) ^ (seed >> (int)input.y) ^ (seed << (int)input.z);
}

uint32_t NoiseGenerator::PCGHash(uint32_t input) {
	uint32_t state = input * 747796405u + 2891336453u;
	uint32_t word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
	return (word >> 22u) ^ word;
}

float NoiseGenerator::RandomFloat(uint32_t& state) {
	state = PCGHash(state);
	return (float)state / (float)std::numeric_limits<uint32_t>::max();
}
glm::vec2 NoiseGenerator::RandomFloat2(uint32_t& state) {
	return { RandomFloat(state), RandomFloat(state) };
}
glm::vec3 NoiseGenerator::RandomFloat3(uint32_t& state) {
	return { RandomFloat(state), RandomFloat(state), RandomFloat(state) };
}

float NoiseGenerator::RandomFloatRange(uint32_t& state, float min, float max) {
	float value = RandomFloat(state);
	return value * (max - min) + min;
}
glm::vec2 NoiseGenerator::RandomFloat2Range(uint32_t& state, float min, float max) {
	return {
		RandomFloatRange(state, min, max),
		RandomFloatRange(state, min, max)
	};
}
glm::vec3 NoiseGenerator::RandomFloat3Range(uint32_t& state, float min, float max) {
	return {
		RandomFloatRange(state, min, max),
		RandomFloatRange(state, min, max),
		RandomFloatRange(state, min, max)
	};
}
