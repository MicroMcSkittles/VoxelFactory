#pragma once
#include "Core/Core.h"
#include "Renderer/Texture.h"

#include <glm/glm.hpp>

class NoiseGenerator {
public:
	static Ref<Texture> GenerateWhiteNoise(int width, int height, uint32_t seed = 0);
	static Ref<Texture> GeneratePerlinNoise(int width, int height, int frequency, const glm::vec2& offset, uint32_t seed);

	static float SampleWhiteNoise(const glm::vec2& position, uint32_t seed);
	static float SamplePerlinNoise2D(const glm::vec2& position, uint32_t seed);
	static float SampleFractalPerlinNoise2D(const glm::vec2& position, int octave_count, uint32_t seed);
	
	static float SamplePerlinNoise3D(const glm::vec3& position, uint32_t seed);
	static float SampleFractalPerlinNoise3D(const glm::vec3& position, int octave_count, uint32_t seed);

private:
	static float CubicInterp(float v1, float v2, float weight);
	static float DotGridGradient2D(const glm::ivec2& gradient_position, const glm::vec2& position, uint32_t seed);
	static glm::vec2 RandomGradient2D(const glm::ivec2& position, uint32_t seed);

	static float DotGridGradient3D(const glm::ivec3& gradient_position, const glm::vec3& position, uint32_t seed);
	static glm::vec3 RandomGradient3D(const glm::ivec3& position, uint32_t seed);

	static uint32_t PCGHash(uint32_t input);
	// Random float between 0 and 1
	static float RandomFloat(uint32_t& state);
	// Random float between min and max
	static float RandomFloatRange(uint32_t& state, float min, float max);
};