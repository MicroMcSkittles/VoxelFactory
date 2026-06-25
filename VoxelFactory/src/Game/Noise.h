#pragma once
#include "Core/Core.h"
#include "Renderer/Texture.h"

#include <glm/glm.hpp>

class NoiseGenerator {
public:
	static void Init();

	static Ref<Texture> GenerateWhiteNoise(int width, int height, uint32_t seed = 0);
	static Ref<Texture> GeneratePerlinNoise(int width, int height, int frequency, const glm::vec2& offset, uint32_t seed = 0);
	static float SamplePerlinNoise(const glm::vec2& position, const glm::vec2& offset, int frequency, uint32_t seed = 0);
private:
	static float CubicInterp(float v1, float v2, float weight);
	static glm::vec2 RandGradient(const glm::vec2& position, uint32_t seed);
	static glm::vec2 RandDirection(uint32_t& state, uint32_t seed);
	static uint32_t PCGHash(uint32_t& state, uint32_t seed);
	static float NormalizedRand(uint32_t& state, uint32_t seed);

};