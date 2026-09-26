#include "Game/Noise.h"
#include <vector>
#include <glad/glad.h>

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

Noise3D::Noise3D(const glm::vec3& size, float resolution) : m_Size(size) {
	m_SampleCount = glm::ceil(m_Size * resolution);
	m_Samples.resize((m_SampleCount.x + 1) * (m_SampleCount.y + 1) * (m_SampleCount.z + 1), 0.0f);
}
Noise3D::Noise3D(const std::vector<float>& samples, const glm::vec3& size, float resolution)
	: m_Size(size), m_Samples(samples)
{
	m_SampleCount = glm::ceil(m_Size * resolution);
}
float Noise3D::GetSample(const glm::vec3& position) const {
	if (position.x < 0.0f || position.y < 0.0f || position.z < 0.0f) return 0.0f;
	if (position.x > m_SampleCount.x || position.y > m_SampleCount.y || position.z > m_SampleCount.z) return 0.0f;

	int index = position.y * ((m_SampleCount.z + 1) * (m_SampleCount.x + 1)) + position.z * (m_SampleCount.x + 1) + position.x;
	return m_Samples[index];
}
float Noise3D::Sample(const glm::vec3& position) const {
	glm::vec3 point = (position * m_SampleCount) / m_Size;
	glm::ivec3 grid_min = { floor(point.x), floor(point.y), floor(point.z) };
	glm::ivec3 grid_max = grid_min + 1;
	glm::vec3 weights = point - glm::vec3(grid_min.x, grid_min.y, grid_min.z);

	// Interpolate top top 2 corners
	float sample_0 = GetSample({ grid_min.x, grid_max.y, grid_min.z });
	float sample_1 = GetSample({ grid_max.x, grid_max.y, grid_min.z });
	float top_0 = NoiseGenerator::CubicInterp(sample_0, sample_1, weights.x);

	// Interpolate top bottom 2 corners
	sample_0 = GetSample({ grid_min.x, grid_max.y, grid_max.z });
	sample_1 = GetSample({ grid_max.x, grid_max.y, grid_max.z });
	float bottom_0 = NoiseGenerator::CubicInterp(sample_0, sample_1, weights.x);

	float top = NoiseGenerator::CubicInterp(top_0, bottom_0, weights.z);

	// Interpolate bottom top 2 corners
	sample_0 = GetSample({ grid_min.x, grid_min.y, grid_min.z });
	sample_1 = GetSample({ grid_max.x, grid_min.y, grid_min.z });
	top_0 = NoiseGenerator::CubicInterp(sample_0, sample_1, weights.x);

	// Interpolate bottom bottom 2 corners
	sample_0 = GetSample({ grid_min.x, grid_min.y, grid_max.z });
	sample_1 = GetSample({ grid_max.x, grid_min.y, grid_max.z });
	bottom_0 = NoiseGenerator::CubicInterp(sample_0, sample_1, weights.x);

	float bottom = NoiseGenerator::CubicInterp(top_0, bottom_0, weights.z);

	return NoiseGenerator::CubicInterp(bottom, top, weights.y);
}

Noise2D::Noise2D(const glm::vec2& size, float resolution) : m_Size(size) {
	m_SampleCount = glm::ceil(m_Size * resolution);
	m_Samples.resize((m_SampleCount.x + 1) * (m_SampleCount.y + 1), 0.0f);
}
Noise2D::Noise2D(const std::vector<float>& samples, const glm::vec2& size, float resolution)
	: m_Size(size), m_Samples(samples)
{
	m_SampleCount = glm::ceil(m_Size * resolution);
}
float Noise2D::GetSample(const glm::vec2& position) const {
	if (position.x < 0.0f || position.y < 0.0f) return 0.0f;
	if (position.x > m_SampleCount.x || position.y > m_SampleCount.y) return 0.0f;

	int index = position.y * (m_SampleCount.x + 1) + position.x;
	return m_Samples[index];
}
float Noise2D::Sample(const glm::vec2& position) const {
	glm::vec2 point = (position * m_SampleCount) / m_Size;
	glm::ivec2 grid_min = { floor(point.x), floor(point.y) };
	glm::ivec2 grid_max = grid_min + 1;
	glm::vec2 weights = point - glm::vec2(grid_min.x, grid_min.y);

	// Interpolate top top 2 corners
	float sample_0 = GetSample({ grid_min.x, grid_min.y });
	float sample_1 = GetSample({ grid_max.x, grid_min.y });
	float top = NoiseGenerator::CubicInterp(sample_0, sample_1, weights.x);

	// Interpolate top bottom 2 corners
	sample_0 = GetSample({ grid_min.x, grid_max.y });
	sample_1 = GetSample({ grid_max.x, grid_max.y });
	float bottom = NoiseGenerator::CubicInterp(sample_0, sample_1, weights.x);

	return NoiseGenerator::CubicInterp(top, bottom, weights.y);
}
