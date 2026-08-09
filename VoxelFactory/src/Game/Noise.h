#pragma once
#include "Core/Core.h"
#include "Renderer/Texture.h"

#include <glm/glm.hpp>

class NoiseGenerator {
public:
	static void Init();

	static Ref<Texture> GenerateWhiteNoise(int width, int height, uint32_t seed = 0);
	static Ref<Texture> GeneratePerlinNoise(int width, int height, int frequency, const glm::vec2& offset, uint32_t seed);

	static float SampleWhiteNoise(const glm::vec2& position, uint32_t seed);
	static float SamplePerlinNoise(const glm::vec2& position, uint32_t seed);
	static float SampleFractalPerlinNoise(const glm::vec2& position, int octave_count, uint32_t seed);

private:
	static float CubicInterp(float v1, float v2, float weight);
	static float DotGridGradient(const glm::ivec2& gradient_position, const glm::vec2& position);
	static glm::vec2 RandomGradient(const glm::ivec2& position);
};