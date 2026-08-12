#include "Game/Voxel.h"

#include <glad/glad.h>

#define TEX_COORD(x,y) y * 16 + x

Block Block::Invalid = Block{ Block::InvalidID };
std::vector<TextureIDs> Block::s_TextureIDs = {
	{ TEX_COORD(10,13) }, // Air/Missing Texture
	{ TEX_COORD(2,0) }, // Dirt
	{ TEX_COORD(3,0), TEX_COORD(0,0), TEX_COORD(2,0) }, // Grass
	{ TEX_COORD(4,4), TEX_COORD(2,4), TEX_COORD(2,0) }, // Snowy Grass
	{ TEX_COORD(4,3) }, // Leaves
	{ TEX_COORD(12,0) }, // Poppy Flower
	{ TEX_COORD(13,0) }, // Dandelion Flower

	{ TEX_COORD(9,13) }, // Bedrock
	{ TEX_COORD(1,0) }, // Stone
	{ TEX_COORD(0,1) }, // Cobble stone
	{ TEX_COORD(1,1) }, // Gravel
	{ TEX_COORD(2,1) }, // Sand
	{ TEX_COORD(0,2) }, // Iron Ore
	{ TEX_COORD(3,3) }, // Copper Ore
	{ TEX_COORD(2,2) }, // Coal Ore
	{ TEX_COORD(1,2) }, // Indium Ore

	{ TEX_COORD(4,1), TEX_COORD(5, 1), TEX_COORD(5,1) }, // Log
	{ TEX_COORD(4,0) }, // Planks
	{ TEX_COORD(3,2), TEX_COORD(4, 0), TEX_COORD(4,0) }, // Bookshelf
	{ TEX_COORD(7,0) }, // Bricks
	{ TEX_COORD(6,3) }, // Stone Bricks
	{ TEX_COORD(6,0) }, // Polished Stone
	{ TEX_COORD(1,3) }, // Glass
	{ TEX_COORD(11,3), TEX_COORD(11,3), TEX_COORD(12,3), TEX_COORD(11,3), TEX_COORD(11,2), TEX_COORD(10,4) }, // Work Bench
	{ TEX_COORD(12,2), TEX_COORD(14,2), TEX_COORD(13,2), TEX_COORD(13,2), TEX_COORD(14,3), TEX_COORD(14,3) }, // Furnace
	
	{ TEX_COORD(11,0) }, // Cobweb
};
std::vector<uint8_t> Block::s_Properties = {
	0,0,0,0,
	BlockProperty_Transparent, // Leaves
	BlockProperty_CrossMesh | BlockProperty_Transparent | BlockProperty_DisableCollision, // Poppy Flower
	BlockProperty_CrossMesh | BlockProperty_Transparent | BlockProperty_DisableCollision, // Dandelion Flower
	BlockProperty_Unbreakable,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	BlockProperty_Glass | BlockProperty_Transparent, // Glass
	0,0,
	BlockProperty_CrossMesh | BlockProperty_Transparent | BlockProperty_DisableCollision, // Cobweb
};

// Normals:
// ( 0, 0, 1 ) = 0
// ( 0, 0,-1 ) = 1
// ( 1, 0, 0 ) = 2
// (-1, 0, 0 ) = 3
// ( 0, 1, 0 ) = 4
// ( 0,-1, 0 ) = 5

// Tex Coords
// ( 0, 0 ) = 0
// ( 1, 0 ) = 1
// ( 0, 1 ) = 2
// ( 1, 1 ) = 3

const BlockVertex Block::CrossMeshVertices[] = {
	{ { -0.5f, -0.5f, -0.5f }, BlockVertex_Normal0 | BlockVertex_TexCoord1 },
	{ {  0.5f, -0.5f,  0.5f }, BlockVertex_Normal0 | BlockVertex_TexCoord0 },
	{ {  0.5f,  0.5f,  0.5f }, BlockVertex_Normal0 | BlockVertex_TexCoord2 },
	{ {  0.5f,  0.5f,  0.5f }, BlockVertex_Normal0 | BlockVertex_TexCoord2 },
	{ { -0.5f,  0.5f, -0.5f }, BlockVertex_Normal0 | BlockVertex_TexCoord3 },
	{ { -0.5f, -0.5f, -0.5f }, BlockVertex_Normal0 | BlockVertex_TexCoord1 },

	{ {  0.5f, -0.5f, -0.5f }, BlockVertex_Normal2 | BlockVertex_TexCoord0 },
	{ {  0.5f,  0.5f, -0.5f }, BlockVertex_Normal2 | BlockVertex_TexCoord2 },
	{ { -0.5f,  0.5f,  0.5f }, BlockVertex_Normal2 | BlockVertex_TexCoord3 },
	{ { -0.5f,  0.5f,  0.5f }, BlockVertex_Normal2 | BlockVertex_TexCoord3 },
	{ { -0.5f, -0.5f,  0.5f }, BlockVertex_Normal2 | BlockVertex_TexCoord1 },
	{ {  0.5f, -0.5f, -0.5f }, BlockVertex_Normal2 | BlockVertex_TexCoord0 }
};
const uint32_t Block::CrossMeshIndices[] = {
	0,  1,   2,
	3,  4,   5,
	2,  1,   0,
	5,  4,   3,
	6,  7,   8,
	9,  10,  11,
	8,  7,   6,
	11, 10,  9
};

const BlockVertex Block::FrontVertices[] = {
	{ { -0.5f, -0.5f,  0.5f }, BlockVertex_Normal0 | BlockVertex_TexCoord1 },
	{ {  0.5f, -0.5f,  0.5f }, BlockVertex_Normal0 | BlockVertex_TexCoord0 },
	{ {  0.5f,  0.5f,  0.5f }, BlockVertex_Normal0 | BlockVertex_TexCoord2 },
	{ {  0.5f,  0.5f,  0.5f }, BlockVertex_Normal0 | BlockVertex_TexCoord2 },
	{ { -0.5f,  0.5f,  0.5f }, BlockVertex_Normal0 | BlockVertex_TexCoord3 },
	{ { -0.5f, -0.5f,  0.5f }, BlockVertex_Normal0 | BlockVertex_TexCoord1 }
};
const BlockVertex Block::BackVertices[] = {
	{ {  0.5f,  0.5f, -0.5f }, BlockVertex_Normal1 | BlockVertex_TexCoord3 },
	{ {  0.5f, -0.5f, -0.5f }, BlockVertex_Normal1 | BlockVertex_TexCoord1 },
	{ { -0.5f, -0.5f, -0.5f }, BlockVertex_Normal1 | BlockVertex_TexCoord0 },
	{ { -0.5f, -0.5f, -0.5f }, BlockVertex_Normal1 | BlockVertex_TexCoord0 },
	{ { -0.5f,  0.5f, -0.5f }, BlockVertex_Normal1 | BlockVertex_TexCoord2 },
	{ {  0.5f,  0.5f, -0.5f }, BlockVertex_Normal1 | BlockVertex_TexCoord3 }
};
const BlockVertex Block::LeftVertices[] = {
	{ {  0.5f, -0.5f, -0.5f }, BlockVertex_Normal2 | BlockVertex_TexCoord0 },
	{ {  0.5f,  0.5f, -0.5f }, BlockVertex_Normal2 | BlockVertex_TexCoord2 },
	{ {  0.5f,  0.5f,  0.5f }, BlockVertex_Normal2 | BlockVertex_TexCoord3 },
	{ {  0.5f,  0.5f,  0.5f }, BlockVertex_Normal2 | BlockVertex_TexCoord3 },
	{ {  0.5f, -0.5f,  0.5f }, BlockVertex_Normal2 | BlockVertex_TexCoord1 },
	{ {  0.5f, -0.5f, -0.5f }, BlockVertex_Normal2 | BlockVertex_TexCoord0 }
};
const BlockVertex Block::RightVertices[] = {
	{ { -0.5f,  0.5f,  0.5f }, BlockVertex_Normal3 | BlockVertex_TexCoord2 },
	{ { -0.5f,  0.5f, -0.5f }, BlockVertex_Normal3 | BlockVertex_TexCoord3 },
	{ { -0.5f, -0.5f, -0.5f }, BlockVertex_Normal3 | BlockVertex_TexCoord1 },
	{ { -0.5f, -0.5f, -0.5f }, BlockVertex_Normal3 | BlockVertex_TexCoord1 },
	{ { -0.5f, -0.5f,  0.5f }, BlockVertex_Normal3 | BlockVertex_TexCoord0 },
	{ { -0.5f,  0.5f,  0.5f }, BlockVertex_Normal3 | BlockVertex_TexCoord2 }
};
const BlockVertex Block::TopVertices[] = {
	{ {  0.5f,  0.5f,  0.5f }, BlockVertex_Normal4 | BlockVertex_TexCoord0 },
	{ {  0.5f,  0.5f, -0.5f }, BlockVertex_Normal4 | BlockVertex_TexCoord2 },
	{ { -0.5f,  0.5f, -0.5f }, BlockVertex_Normal4 | BlockVertex_TexCoord3 },
	{ { -0.5f,  0.5f, -0.5f }, BlockVertex_Normal4 | BlockVertex_TexCoord3 },
	{ { -0.5f,  0.5f,  0.5f }, BlockVertex_Normal4 | BlockVertex_TexCoord1 },
	{ {  0.5f,  0.5f,  0.5f }, BlockVertex_Normal4 | BlockVertex_TexCoord0 }
};
const BlockVertex Block::BottomVertices[] = {
	{ { -0.5f, -0.5f, -0.5f }, BlockVertex_Normal5 | BlockVertex_TexCoord3 },
	{ {  0.5f, -0.5f, -0.5f }, BlockVertex_Normal5 | BlockVertex_TexCoord2 },
	{ {  0.5f, -0.5f,  0.5f }, BlockVertex_Normal5 | BlockVertex_TexCoord0 },
	{ {  0.5f, -0.5f,  0.5f }, BlockVertex_Normal5 | BlockVertex_TexCoord0 },
	{ { -0.5f, -0.5f,  0.5f }, BlockVertex_Normal5 | BlockVertex_TexCoord1 },
	{ { -0.5f, -0.5f, -0.5f }, BlockVertex_Normal5 | BlockVertex_TexCoord3 }
};
const BlockVertex Block::Vertices[] = {
	// Front
	{ { -0.5f, -0.5f,  0.5f }, BlockVertex_Normal0 | BlockVertex_TexCoord1 },
	{ {  0.5f, -0.5f,  0.5f }, BlockVertex_Normal0 | BlockVertex_TexCoord0 },
	{ {  0.5f,  0.5f,  0.5f }, BlockVertex_Normal0 | BlockVertex_TexCoord2 },
	{ {  0.5f,  0.5f,  0.5f }, BlockVertex_Normal0 | BlockVertex_TexCoord2 },
	{ { -0.5f,  0.5f,  0.5f }, BlockVertex_Normal0 | BlockVertex_TexCoord3 },
	{ { -0.5f, -0.5f,  0.5f }, BlockVertex_Normal0 | BlockVertex_TexCoord1 },

	// Back
	{ {  0.5f,  0.5f, -0.5f }, BlockVertex_Normal1 | BlockVertex_TexCoord3 },
	{ {  0.5f, -0.5f, -0.5f }, BlockVertex_Normal1 | BlockVertex_TexCoord1 },
	{ { -0.5f, -0.5f, -0.5f }, BlockVertex_Normal1 | BlockVertex_TexCoord0 },
	{ { -0.5f, -0.5f, -0.5f }, BlockVertex_Normal1 | BlockVertex_TexCoord0 },
	{ { -0.5f,  0.5f, -0.5f }, BlockVertex_Normal1 | BlockVertex_TexCoord2 },
	{ {  0.5f,  0.5f, -0.5f }, BlockVertex_Normal1 | BlockVertex_TexCoord3 },

	// Left
	{ {  0.5f, -0.5f, -0.5f }, BlockVertex_Normal2 | BlockVertex_TexCoord0 },
	{ {  0.5f,  0.5f, -0.5f }, BlockVertex_Normal2 | BlockVertex_TexCoord2 },
	{ {  0.5f,  0.5f,  0.5f }, BlockVertex_Normal2 | BlockVertex_TexCoord3 },
	{ {  0.5f,  0.5f,  0.5f }, BlockVertex_Normal2 | BlockVertex_TexCoord3 },
	{ {  0.5f, -0.5f,  0.5f }, BlockVertex_Normal2 | BlockVertex_TexCoord1 },
	{ {  0.5f, -0.5f, -0.5f }, BlockVertex_Normal2 | BlockVertex_TexCoord0 },

	// Right
	{ { -0.5f,  0.5f,  0.5f }, BlockVertex_Normal3 | BlockVertex_TexCoord2 },
	{ { -0.5f,  0.5f, -0.5f }, BlockVertex_Normal3 | BlockVertex_TexCoord3 },
	{ { -0.5f, -0.5f, -0.5f }, BlockVertex_Normal3 | BlockVertex_TexCoord1 },
	{ { -0.5f, -0.5f, -0.5f }, BlockVertex_Normal3 | BlockVertex_TexCoord1 },
	{ { -0.5f, -0.5f,  0.5f }, BlockVertex_Normal3 | BlockVertex_TexCoord0 },
	{ { -0.5f,  0.5f,  0.5f }, BlockVertex_Normal3 | BlockVertex_TexCoord2 },

	// Top
	{ {  0.5f,  0.5f,  0.5f }, BlockVertex_Normal4 | BlockVertex_TexCoord0 },
	{ {  0.5f,  0.5f, -0.5f }, BlockVertex_Normal4 | BlockVertex_TexCoord2 },
	{ { -0.5f,  0.5f, -0.5f }, BlockVertex_Normal4 | BlockVertex_TexCoord3 },
	{ { -0.5f,  0.5f, -0.5f }, BlockVertex_Normal4 | BlockVertex_TexCoord3 },
	{ { -0.5f,  0.5f,  0.5f }, BlockVertex_Normal4 | BlockVertex_TexCoord1 },
	{ {  0.5f,  0.5f,  0.5f }, BlockVertex_Normal4 | BlockVertex_TexCoord0 },

	// Bottom
	{ { -0.5f, -0.5f, -0.5f }, BlockVertex_Normal5 | BlockVertex_TexCoord3 },
	{ {  0.5f, -0.5f, -0.5f }, BlockVertex_Normal5 | BlockVertex_TexCoord2 },
	{ {  0.5f, -0.5f,  0.5f }, BlockVertex_Normal5 | BlockVertex_TexCoord0 },
	{ {  0.5f, -0.5f,  0.5f }, BlockVertex_Normal5 | BlockVertex_TexCoord0 },
	{ { -0.5f, -0.5f,  0.5f }, BlockVertex_Normal5 | BlockVertex_TexCoord1 },
	{ { -0.5f, -0.5f, -0.5f }, BlockVertex_Normal5 | BlockVertex_TexCoord3 }
};
const uint32_t Block::FaceIndices[] = {
	0,  1,  2,
	3,  4,  5
};
const uint32_t Block::Indices[] = {
	0,  1,  2,  3,  4,  5,  // Front
	6,  7,  8,  9,  10, 11, // Back
	12, 13, 14, 15, 16, 17, // Left
	18, 19, 20, 21, 22, 23, // Right
	24, 25, 26, 27, 28, 29, // Top
	30, 31, 32, 33, 34, 35  // Bottom
};

const VertexLayout Block::Layout = { {
	{ GL_FLOAT, 3 }, // a_Pos
	{ GL_UNSIGNED_INT, 1 }, // a_Data
	{ GL_UNSIGNED_INT, 1 }, // a_TextureID
} };

bool Block::HasOrientation(uint8_t id) {
	uint8_t actual_id = id & 0b00111111;
	if (actual_id == 5 || actual_id == 18 || actual_id == 19) return true;
	return false;
}
int Block::GetAxisCount(uint8_t id) {
	if (id == 5) return 3; // 3 possable orentations
	if (id == 18 || id == 19) return 4; // 4 possable orentations
	return 0;
}
uint8_t Block::CalculateOrientation(const glm::vec3& direction, uint8_t id) {
	glm::vec3 orientation = glm::vec3(0.0f);
	if (abs(direction.x) > abs(direction.y) && abs(direction.x) > abs(direction.z)) orientation = glm::vec3(direction.x, 0.0f, 0.0f);
	else if (abs(direction.z) > abs(direction.y)) orientation = glm::vec3(0.0f, 0.0f, direction.z);
	else orientation = glm::vec3(0.0f, direction.y, 0.0f);
	orientation = glm::normalize(orientation);

	int axis_count = GetAxisCount(id);

	if (axis_count == 3) {
		if (orientation.x != 0) return 0b00000000;
		if (orientation.y != 0) return 0b01000000;
		if (orientation.z != 0) return 0b10000000;
	}
	if (axis_count == 4) {
		if (orientation.x > 0) return 0b00000000;
		if (orientation.z > 0) return 0b01000000;
		if (orientation.x < 0) return 0b10000000;
		if (orientation.z < 0) return 0b11000000;
	}

	return 0;
}

uint32_t Block::OrientVector(uint32_t direction, int axis_count, uint8_t orientation) {
	// TODO: dont over complicate this
	if (orientation == 1) return direction;

	glm::vec3 vector_dir = glm::vec3(0.0f);
	switch (direction) {
	case BlockVertex_Normal0: vector_dir = {  0.0f,  0.0f,  1.0f }; break;
	case BlockVertex_Normal1: vector_dir = {  0.0f,  0.0f, -1.0f }; break;
	case BlockVertex_Normal2: vector_dir = {  1.0f,  0.0f,  0.0f }; break;
	case BlockVertex_Normal3: vector_dir = { -1.0f,  0.0f,  0.0f }; break;
	case BlockVertex_Normal4: vector_dir = {  0.0f,  1.0f,  0.0f }; break;
	case BlockVertex_Normal5: vector_dir = {  0.0f, -1.0f,  0.0f }; break;
	}

	vector_dir = OrientVector(vector_dir, axis_count, orientation);

	if (vector_dir == glm::vec3( 0.0f,  0.0f,  1.0f)) return BlockVertex_Normal0;
	if (vector_dir == glm::vec3( 0.0f,  0.0f, -1.0f)) return BlockVertex_Normal1;
	if (vector_dir == glm::vec3( 1.0f,  0.0f,  0.0f)) return BlockVertex_Normal2;
	if (vector_dir == glm::vec3(-1.0f,  0.0f,  0.0f)) return BlockVertex_Normal3;
	if (vector_dir == glm::vec3( 0.0f,  1.0f,  0.0f)) return BlockVertex_Normal4;
	if (vector_dir == glm::vec3( 0.0f, -1.0f,  0.0f)) return BlockVertex_Normal5;

	return 0;
}

const TextureIDs& Block::GetTextureIDs(uint8_t id) {
	uint8_t real_id = id & OrientationMask;
	if (real_id >= BlockID_Count) return s_TextureIDs[0]; // return missing texture
	return s_TextureIDs[real_id];
}
uint8_t Block::GetProperties(uint8_t id) {
	uint8_t real_id = id & OrientationMask;
	if (real_id >= BlockID_Count) return 0;
	return s_Properties[real_id];
}
bool Block::HasProperty(uint8_t id, uint8_t property) {
	uint8_t real_id = id & OrientationMask;
	if (real_id >= BlockID_Count) return false;
	return s_Properties[real_id] & property;
}

glm::vec3 Block::OrientVector(const glm::vec3& direction, int axis_count, uint8_t orientation) {
	if (axis_count == 3) {
		if (orientation == 0) return glm::vec3(-direction.y, direction.x, direction.z);
		if (orientation == 1) return direction;
		if (orientation == 2) return glm::vec3(direction.x, -direction.z, direction.y);
	}
	if (axis_count == 4) {
		if (orientation == 0) return glm::vec3(direction.z, direction.y, -direction.x);
		if (orientation == 1) return direction;
		if (orientation == 3) return glm::vec3(-direction.x, direction.y, -direction.z);
		if (orientation == 2) return glm::vec3(-direction.z, direction.y, direction.x);
	}

	return direction;
}
