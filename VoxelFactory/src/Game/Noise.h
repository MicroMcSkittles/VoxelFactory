#pragma once
#include "Core/Core.h"
#include "Renderer/Texture.h"

#include <glm/glm.hpp>

class Noise2D {
public:
	Noise2D() : m_SampleCount(0.0f), m_Size(0.0f) {}
	Noise2D(const glm::vec2& size, float resolution);
	Noise2D(const std::vector<float>& samples, const glm::vec2& size, float resolution);
	~Noise2D() {}

	std::vector<float>& GetSamples() { return m_Samples; }
	const glm::vec2& GetSampleCount() const { return m_SampleCount; }
	const glm::vec2& GetSize()        const { return m_Size; }

	float Sample(const glm::vec2& position) const;

private:
	float GetSample(const glm::vec2& position) const;

private:
	std::vector<float> m_Samples;
	glm::vec2 m_SampleCount;
	glm::vec2 m_Size;
};
class Noise3D {
public:
	Noise3D(): m_SampleCount(0.0f), m_Size(0.0f) { }
	Noise3D(const glm::vec3& size, float resolution);
	Noise3D(const std::vector<float>& samples, const glm::vec3& size, float resolution);
	~Noise3D() { }

	std::vector<float>& GetSamples()        { return m_Samples; }
	const glm::vec3& GetSampleCount() const { return m_SampleCount; }
	const glm::vec3& GetSize()        const { return m_Size; }

	float Sample(const glm::vec3& position) const;

private:
	float GetSample(const glm::vec3& position) const;

private:
	std::vector<float> m_Samples;
	glm::vec3 m_SampleCount;
	glm::vec3 m_Size;
};

class NoiseGenerator {
public:

	static float SamplePerlinNoise2D(const glm::vec2& position, uint32_t seed);
	static float SampleFractalPerlinNoise2D(const glm::vec2& position, int octave_count, uint32_t seed);
	
	static float SamplePerlinNoise3D(const glm::vec3& position, uint32_t seed);
	static float SampleFractalPerlinNoise3D(const glm::vec3& position, int octave_count, uint32_t seed);

	static uint32_t State(const glm::vec3& input, uint32_t seed);
	static uint32_t PCGHash(uint32_t input);

	// Random float between 0 and 1
	static float     RandomFloat(uint32_t&  state);
	static glm::vec2 RandomFloat2(uint32_t& state);
	static glm::vec3 RandomFloat3(uint32_t& state);

	// Random float between min and max
	static float     RandomFloatRange(uint32_t& state,  float min, float max);
	static glm::vec2 RandomFloat2Range(uint32_t& state, float min, float max);
	static glm::vec3 RandomFloat3Range(uint32_t& state, float min, float max);

	static float CubicInterp(float v1, float v2, float weight);

private:
	static float DotGridGradient2D(const glm::ivec2& gradient_position, const glm::vec2& position, uint32_t seed);
	static glm::vec2 RandomGradient2D(const glm::ivec2& position, uint32_t seed);

	static float DotGridGradient3D(const glm::ivec3& gradient_position, const glm::vec3& position, uint32_t seed);
	static glm::vec3 RandomGradient3D(const glm::ivec3& position, uint32_t seed);

};