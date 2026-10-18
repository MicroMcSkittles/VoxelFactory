#pragma once
#include "Core/Core.h"
#include "Core/Utils.h"
#include "Renderer/Buffers.h"
#include <glm/glm.hpp>

enum BlockID {
	BlockID_Air = 0,

	// Flora
	BlockID_Dirt,
	BlockID_Grass,
	BlockID_SnowyGrass,
	BlockID_Leaves,
	BlockID_PoppyFlower,
	BlockID_DandelionFlower,
	BlockID_ShortGrass,

	// Stone
	BlockID_Bedrock,
	BlockID_Stone,
	BlockID_CobbleStone,
	BlockID_Gravel,
	BlockID_Sand,
	BlockID_IronOre,
	BlockID_CopperOre,
	BlockID_CoalOre,
	BlockID_IndiumOre,

	// Building
	BlockID_Log,
	BlockID_Plank,
	BlockID_Bookshelf,
	BlockID_Bricks,
	BlockID_StoneBricks,
	BlockID_PolishedStone,
	BlockID_Glass,
	BlockID_WorkBench,
	BlockID_Furnace,

	BlockID_Glowstone,

	// Misc
	BlockID_Cobweb,
	BlockID_Count
};

enum BlockVertex_Normal {
	BlockVertex_Normal0 = 0 << 24, // ( 0, 0, 1 )
	BlockVertex_Normal1 = 1 << 24, // ( 0, 0,-1 )
	BlockVertex_Normal2 = 2 << 24, // ( 1, 0, 0 )
	BlockVertex_Normal3 = 3 << 24, // (-1, 0, 0 )
	BlockVertex_Normal4 = 4 << 24, // ( 0, 1, 0 )
	BlockVertex_Normal5 = 5 << 24  // ( 0,-1, 0 )
};
enum BlockVertex_TexCoord {
	BlockVertex_TexCoord0 = 0 << 27, // ( 0, 0 )
	BlockVertex_TexCoord1 = 1 << 27, // ( 1, 0 )
	BlockVertex_TexCoord2 = 2 << 27, // ( 0, 1 )
	BlockVertex_TexCoord3 = 3 << 27  // ( 1, 1 )
};

const inline static uint8_t MaxLightLevel = 15;

struct BlockVertex {
	uint32_t position; // x: 0-7; y: 8-23; z: 24-31
	uint32_t data; // id: 0-19; light_level: 20-23 normal: 24,25,26; tex_coord: 27, 28; ambient_occlusion: 29, 30

	static uint32_t PackPosition(const glm::vec3& position);
	static glm::vec3 UnpackPosition(uint32_t position_bits);
	const inline static uint32_t DataOffset = 24;
};

struct TextureIDs {
	uint32_t front;
	uint32_t back;

	uint32_t left;
	uint32_t right;

	uint32_t top;
	uint32_t bottom;

	inline std::vector<uint32_t> List() const { return { front, back, left, right, top, bottom }; }

	TextureIDs() : front(0), back(0), left(0), right(0), top(0), bottom(0) {}
	TextureIDs(uint32_t id) : front(id), back(id), left(id), right(id), top(id), bottom(id) {}
	TextureIDs(uint32_t sides, uint32_t top, uint32_t bottom) : front(sides), back(sides), left(sides), right(sides), top(top), bottom(bottom) {}
	TextureIDs(uint32_t front, uint32_t back, uint32_t left, uint32_t right, uint32_t top, uint32_t bottom) : front(front), back(back), left(left), right(right), top(top), bottom(bottom) {}
};

enum BlockProperties {
	BlockProperty_Transparent          = BIT(0),
	BlockProperty_Glass                = BIT(1),
	BlockProperty_CrossMesh            = BIT(2),
	BlockProperty_DisableCollision     = BIT(3),
	BlockProperty_Unbreakable          = BIT(4),
	BlockProperty_HasOrientation       = BIT(5),
	BlockProperty_HasOrientation3Axis  = BIT(5) | BIT(6),
	BlockProperty_HasOrientation4Axis  = BIT(5) | BIT(7),
	BlockProperty_LightEmitting        = BIT(8)
};
struct BlockData {
	std::string name;
	TextureIDs texture_ids;
	uint16_t properties = 0;
	float break_time = 1.0f;
};

struct Block {
	uint8_t id = 0;

	static int GetAxisCount(uint8_t id);
	static uint8_t CalculateOrientation(const glm::vec3& direction, uint8_t id);
	static glm::vec3 OrientVector(const glm::vec3& direction, int axis_count, uint8_t orientation);
	static uint32_t OrientVector(uint32_t direction, int axis_count, uint8_t orientation);

	static void LoadData(const std::string& filename);

	static const TextureIDs& GetBreakTextureIDs(uint8_t state);
	static const TextureIDs& GetTextureIDs(uint8_t id);
	static uint16_t GetProperties(uint8_t id);
	static bool HasProperty(uint8_t id, uint16_t property);
	static const BlockData& GetData(uint8_t id);

	const inline static uint8_t InvalidID = std::numeric_limits<uint8_t>::max();
	static Block Invalid;

	const inline static uint8_t OrientationMask = 0b11000000;

	const static BlockVertex CrossMeshVertices[];
	const static uint32_t CrossMeshIndices[];
	
	const inline static size_t FaceVertexCount = 6;
	const inline static size_t FaceIndexCount = 6;

	const static BlockVertex FrontVertices[];
	const static BlockVertex BackVertices[];
	const static BlockVertex LeftVertices[];
	const static BlockVertex RightVertices[];
	const static BlockVertex TopVertices[];
	const static BlockVertex BottomVertices[];
	const static uint32_t FaceIndices[];

	const static BlockVertex Vertices[];
	const static uint32_t Indices[];

	const static VertexLayout Layout;
	
private:
	inline static TextureIDs s_MissingTexture;
	inline static std::vector<TextureIDs> s_BreakTextureIDs;
	inline static std::vector<BlockData> s_Data;
};