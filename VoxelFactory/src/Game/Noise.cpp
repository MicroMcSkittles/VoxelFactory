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
			float value = SampleFractalPerlinNoise({ (float)x / (float)width,(float)y / (float)height }, 12, seed);
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

float NoiseGenerator::SamplePerlinNoise(const glm::vec2& position, uint32_t seed) {
	glm::ivec2 grid_min = { (int)position.x, (int)position.y };
	glm::ivec2 grid_max = grid_min + 1;

	glm::vec2 weights = {
		position.x - (float)grid_min.x,
		position.y - (float)grid_min.y
	};

	// Interpolate top 2 corners
	float corner_0 = DotGridGradient(grid_min, position);
	float corner_1 = DotGridGradient({ grid_max.x, grid_min.y }, position);
	float top = CubicInterp(corner_0, corner_1, weights.x);

	// Interpolate bottom 2 corners
	corner_0 = DotGridGradient({ grid_min.x, grid_max.y }, position);
	corner_1 = DotGridGradient(grid_max, position);
	float bottom = CubicInterp(corner_0, corner_1, weights.x);

	return CubicInterp(top, bottom, weights.y);
}

float NoiseGenerator::SampleFractalPerlinNoise(const glm::vec2& position, int octave_count, uint32_t seed)
{
	float value = 0.0f;

	float frequency = 1.0f;
	float amplitude = 1.0f;
	for (int i = 0; i < octave_count; i++) {
		value += SamplePerlinNoise({ position.x * frequency, position.y * frequency }, seed) * amplitude;
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
float NoiseGenerator::DotGridGradient(const glm::ivec2& gradient_position, const glm::vec2& position) {
	glm::vec2 gradient = RandomGradient(gradient_position);
	glm::vec2 distance = position - glm::vec2((float)gradient_position.x, (float)gradient_position.y);
	return glm::dot(distance, gradient);
}
glm::vec2 NoiseGenerator::RandomGradient(const glm::ivec2& position) {
	const uint32_t w = 8 * sizeof(uint32_t);
	const uint32_t s = w / 2;
	uint32_t a = position.x;
	uint32_t b = position.y;
	a *= 3284157443;

	b ^= a << s | a >> w - s;
	b *= 1911520717;

	a ^= b << s | b >> w - s;
	a *= 2048419325;
	float random = a * (PI / ~(~0u >> 1));
	if (random > PI2) random -= PI2;
	else if (random < 0.0f) random += PI2;

	return {
		cos(random),
		sin(random)
	};
}