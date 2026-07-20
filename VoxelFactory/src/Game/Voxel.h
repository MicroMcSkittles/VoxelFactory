#pragma once
#include "Core/Core.h"
#include "Renderer/Buffers.h"
#include <glm/glm.hpp>

struct BlockVertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 tex_coord;
	float ambient_occlusion;
	uint32_t id;
};

struct TextureIDs {
	uint32_t front;
	uint32_t back;

	uint32_t left;
	uint32_t right;

	uint32_t top;
	uint32_t bottom;

	TextureIDs(uint32_t id) : front(id), back(id), left(id), right(id), top(id), bottom(id) {}
	TextureIDs(uint32_t sides, uint32_t top, uint32_t bottom) : front(sides), back(sides), left(sides), right(sides), top(top), bottom(bottom) {}
	TextureIDs(uint32_t front, uint32_t back, uint32_t left, uint32_t right, uint32_t top, uint32_t bottom) : front(front), back(back), left(left), right(right), top(top), bottom(bottom) {}
};

struct Block {
	uint8_t id;

	static void SetMeshType(uint8_t id);

	static bool HasOrientation(uint8_t id);
	static int GetAxisCount(uint8_t id);
	static uint8_t CalculateOrientation(const glm::vec3& direction, uint8_t id);
	static glm::vec3 OrientVector(const glm::vec3& direction, int axis_count, uint8_t orientation);

	static std::vector<TextureIDs> BlockTextureIDs;

	inline static Ref<VertexArray> Mesh = nullptr;

	const inline static uint8_t InvalidID = std::numeric_limits<uint8_t>::max();
	static Block Invalid;

	const inline static size_t FaceVertexCount = 6;
	const inline static size_t FaceIndexCount = 6;

	const static BlockVertex FrontVertices[];
	const static BlockVertex BackVertices[];
	const static BlockVertex LeftVertices[];
	const static BlockVertex RightVertices[];
	const static BlockVertex TopVertices[];
	const static BlockVertex BottomVertices[];
	const static BlockVertex Vertices[];

	const static uint32_t FaceIndices[];
	const static uint32_t Indices[];


	const static VertexLayout Layout;
};