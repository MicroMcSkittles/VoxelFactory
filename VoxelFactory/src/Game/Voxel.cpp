#include "Game/Voxel.h"
#include "Core/JSON.h"

#include <glad/glad.h>

#define TEX_COORD_IMPL(v) v.y * 16 + v.x
#define TEX_COORD(v) TEX_COORD_IMPL(((glm::vec2)v))

Block Block::Invalid = Block{ Block::InvalidID };

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
	{ BlockVertex::PackPosition({ 0.0f, 0.0f, 0.0f }), BlockVertex_Normal0 | BlockVertex_TexCoord1 },
	{ BlockVertex::PackPosition({ 1.0f, 0.0f, 1.0f }), BlockVertex_Normal0 | BlockVertex_TexCoord0 },
	{ BlockVertex::PackPosition({ 1.0f, 1.0f, 1.0f }), BlockVertex_Normal0 | BlockVertex_TexCoord2 },
	{ BlockVertex::PackPosition({ 1.0f, 1.0f, 1.0f }), BlockVertex_Normal0 | BlockVertex_TexCoord2 },
	{ BlockVertex::PackPosition({ 0.0f, 1.0f, 0.0f }), BlockVertex_Normal0 | BlockVertex_TexCoord3 },
	{ BlockVertex::PackPosition({ 0.0f, 0.0f, 0.0f }), BlockVertex_Normal0 | BlockVertex_TexCoord1 },

	{ BlockVertex::PackPosition({ 1.0f, 0.0f, 0.0f }), BlockVertex_Normal2 | BlockVertex_TexCoord0 },
	{ BlockVertex::PackPosition({ 1.0f, 1.0f, 0.0f }), BlockVertex_Normal2 | BlockVertex_TexCoord2 },
	{ BlockVertex::PackPosition({ 0.0f, 1.0f, 1.0f }), BlockVertex_Normal2 | BlockVertex_TexCoord3 },
	{ BlockVertex::PackPosition({ 0.0f, 1.0f, 1.0f }), BlockVertex_Normal2 | BlockVertex_TexCoord3 },
	{ BlockVertex::PackPosition({ 0.0f, 0.0f, 1.0f }), BlockVertex_Normal2 | BlockVertex_TexCoord1 },
	{ BlockVertex::PackPosition({ 1.0f, 0.0f, 0.0f }), BlockVertex_Normal2 | BlockVertex_TexCoord0 }
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
	{ BlockVertex::PackPosition({ 0.0f, 0.0f, 1.0f }), BlockVertex_Normal0 | BlockVertex_TexCoord1 },
	{ BlockVertex::PackPosition({ 1.0f, 0.0f, 1.0f }), BlockVertex_Normal0 | BlockVertex_TexCoord0 },
	{ BlockVertex::PackPosition({ 1.0f, 1.0f, 1.0f }), BlockVertex_Normal0 | BlockVertex_TexCoord2 },
	{ BlockVertex::PackPosition({ 1.0f, 1.0f, 1.0f }), BlockVertex_Normal0 | BlockVertex_TexCoord2 },
	{ BlockVertex::PackPosition({ 0.0f, 1.0f, 1.0f }), BlockVertex_Normal0 | BlockVertex_TexCoord3 },
	{ BlockVertex::PackPosition({ 0.0f, 0.0f, 1.0f }), BlockVertex_Normal0 | BlockVertex_TexCoord1 }
};
const BlockVertex Block::BackVertices[] = {
	{ BlockVertex::PackPosition({ 1.0f, 1.0f, 0.0f }), BlockVertex_Normal1 | BlockVertex_TexCoord3 },
	{ BlockVertex::PackPosition({ 1.0f, 0.0f, 0.0f }), BlockVertex_Normal1 | BlockVertex_TexCoord1 },
	{ BlockVertex::PackPosition({ 0.0f, 0.0f, 0.0f }), BlockVertex_Normal1 | BlockVertex_TexCoord0 },
	{ BlockVertex::PackPosition({ 0.0f, 0.0f, 0.0f }), BlockVertex_Normal1 | BlockVertex_TexCoord0 },
	{ BlockVertex::PackPosition({ 0.0f, 1.0f, 0.0f }), BlockVertex_Normal1 | BlockVertex_TexCoord2 },
	{ BlockVertex::PackPosition({ 1.0f, 1.0f, 0.0f }), BlockVertex_Normal1 | BlockVertex_TexCoord3 }
};
const BlockVertex Block::LeftVertices[] = {
	{ BlockVertex::PackPosition({ 1.0f, 0.0f, 0.0f }), BlockVertex_Normal2 | BlockVertex_TexCoord0 },
	{ BlockVertex::PackPosition({ 1.0f, 1.0f, 0.0f }), BlockVertex_Normal2 | BlockVertex_TexCoord2 },
	{ BlockVertex::PackPosition({ 1.0f, 1.0f, 1.0f }), BlockVertex_Normal2 | BlockVertex_TexCoord3 },
	{ BlockVertex::PackPosition({ 1.0f, 1.0f, 1.0f }), BlockVertex_Normal2 | BlockVertex_TexCoord3 },
	{ BlockVertex::PackPosition({ 1.0f, 0.0f, 1.0f }), BlockVertex_Normal2 | BlockVertex_TexCoord1 },
	{ BlockVertex::PackPosition({ 1.0f, 0.0f, 0.0f }), BlockVertex_Normal2 | BlockVertex_TexCoord0 }
};
const BlockVertex Block::RightVertices[] = {
	{ BlockVertex::PackPosition({ 0.0f, 1.0f, 1.0f }), BlockVertex_Normal3 | BlockVertex_TexCoord2 },
	{ BlockVertex::PackPosition({ 0.0f, 1.0f, 0.0f }), BlockVertex_Normal3 | BlockVertex_TexCoord3 },
	{ BlockVertex::PackPosition({ 0.0f, 0.0f, 0.0f }), BlockVertex_Normal3 | BlockVertex_TexCoord1 },
	{ BlockVertex::PackPosition({ 0.0f, 0.0f, 0.0f }), BlockVertex_Normal3 | BlockVertex_TexCoord1 },
	{ BlockVertex::PackPosition({ 0.0f, 0.0f, 1.0f }), BlockVertex_Normal3 | BlockVertex_TexCoord0 },
	{ BlockVertex::PackPosition({ 0.0f, 1.0f, 1.0f }), BlockVertex_Normal3 | BlockVertex_TexCoord2 }
};
const BlockVertex Block::TopVertices[] = {
	{ BlockVertex::PackPosition({ 1.0f, 1.0f, 1.0f }), BlockVertex_Normal4 | BlockVertex_TexCoord0 },
	{ BlockVertex::PackPosition({ 1.0f, 1.0f, 0.0f }), BlockVertex_Normal4 | BlockVertex_TexCoord2 },
	{ BlockVertex::PackPosition({ 0.0f, 1.0f, 0.0f }), BlockVertex_Normal4 | BlockVertex_TexCoord3 },
	{ BlockVertex::PackPosition({ 0.0f, 1.0f, 0.0f }), BlockVertex_Normal4 | BlockVertex_TexCoord3 },
	{ BlockVertex::PackPosition({ 0.0f, 1.0f, 1.0f }), BlockVertex_Normal4 | BlockVertex_TexCoord1 },
	{ BlockVertex::PackPosition({ 1.0f, 1.0f, 1.0f }), BlockVertex_Normal4 | BlockVertex_TexCoord0 }
};
const BlockVertex Block::BottomVertices[] = {
	{ BlockVertex::PackPosition({ 0.0f, 0.0f, 0.0f }), BlockVertex_Normal5 | BlockVertex_TexCoord3 },
	{ BlockVertex::PackPosition({ 1.0f, 0.0f, 0.0f }), BlockVertex_Normal5 | BlockVertex_TexCoord2 },
	{ BlockVertex::PackPosition({ 1.0f, 0.0f, 1.0f }), BlockVertex_Normal5 | BlockVertex_TexCoord0 },
	{ BlockVertex::PackPosition({ 1.0f, 0.0f, 1.0f }), BlockVertex_Normal5 | BlockVertex_TexCoord0 },
	{ BlockVertex::PackPosition({ 0.0f, 0.0f, 1.0f }), BlockVertex_Normal5 | BlockVertex_TexCoord1 },
	{ BlockVertex::PackPosition({ 0.0f, 0.0f, 0.0f }), BlockVertex_Normal5 | BlockVertex_TexCoord3 }
};
const BlockVertex Block::Vertices[] = {
	// Front
	{ BlockVertex::PackPosition({ 0.0f, 0.0f, 1.0f }), BlockVertex_Normal0 | BlockVertex_TexCoord1 },
	{ BlockVertex::PackPosition({ 1.0f, 0.0f, 1.0f }), BlockVertex_Normal0 | BlockVertex_TexCoord0 },
	{ BlockVertex::PackPosition({ 1.0f, 1.0f, 1.0f }), BlockVertex_Normal0 | BlockVertex_TexCoord2 },
	{ BlockVertex::PackPosition({ 1.0f, 1.0f, 1.0f }), BlockVertex_Normal0 | BlockVertex_TexCoord2 },
	{ BlockVertex::PackPosition({ 0.0f, 1.0f, 1.0f }), BlockVertex_Normal0 | BlockVertex_TexCoord3 },
	{ BlockVertex::PackPosition({ 0.0f, 0.0f, 1.0f }), BlockVertex_Normal0 | BlockVertex_TexCoord1 },

	// Back
	{ BlockVertex::PackPosition({ 1.0f, 1.0f, 0.0f }), BlockVertex_Normal1 | BlockVertex_TexCoord3 },
	{ BlockVertex::PackPosition({ 1.0f, 0.0f, 0.0f }), BlockVertex_Normal1 | BlockVertex_TexCoord1 },
	{ BlockVertex::PackPosition({ 0.0f, 0.0f, 0.0f }), BlockVertex_Normal1 | BlockVertex_TexCoord0 },
	{ BlockVertex::PackPosition({ 0.0f, 0.0f, 0.0f }), BlockVertex_Normal1 | BlockVertex_TexCoord0 },
	{ BlockVertex::PackPosition({ 0.0f, 1.0f, 0.0f }), BlockVertex_Normal1 | BlockVertex_TexCoord2 },
	{ BlockVertex::PackPosition({ 1.0f, 1.0f, 0.0f }), BlockVertex_Normal1 | BlockVertex_TexCoord3 },

	// Left
	{ BlockVertex::PackPosition({ 1.0f, 0.0f, 0.0f }), BlockVertex_Normal2 | BlockVertex_TexCoord0 },
	{ BlockVertex::PackPosition({ 1.0f, 1.0f, 0.0f }), BlockVertex_Normal2 | BlockVertex_TexCoord2 },
	{ BlockVertex::PackPosition({ 1.0f, 1.0f, 1.0f }), BlockVertex_Normal2 | BlockVertex_TexCoord3 },
	{ BlockVertex::PackPosition({ 1.0f, 1.0f, 1.0f }), BlockVertex_Normal2 | BlockVertex_TexCoord3 },
	{ BlockVertex::PackPosition({ 1.0f, 0.0f, 1.0f }), BlockVertex_Normal2 | BlockVertex_TexCoord1 },
	{ BlockVertex::PackPosition({ 1.0f, 0.0f, 0.0f }), BlockVertex_Normal2 | BlockVertex_TexCoord0 },

	// Right
	{ BlockVertex::PackPosition({ 0.0f, 1.0f, 1.0f }), BlockVertex_Normal3 | BlockVertex_TexCoord2 },
	{ BlockVertex::PackPosition({ 0.0f, 1.0f, 0.0f }), BlockVertex_Normal3 | BlockVertex_TexCoord3 },
	{ BlockVertex::PackPosition({ 0.0f, 0.0f, 0.0f }), BlockVertex_Normal3 | BlockVertex_TexCoord1 },
	{ BlockVertex::PackPosition({ 0.0f, 0.0f, 0.0f }), BlockVertex_Normal3 | BlockVertex_TexCoord1 },
	{ BlockVertex::PackPosition({ 0.0f, 0.0f, 1.0f }), BlockVertex_Normal3 | BlockVertex_TexCoord0 },
	{ BlockVertex::PackPosition({ 0.0f, 1.0f, 1.0f }), BlockVertex_Normal3 | BlockVertex_TexCoord2 },

	// Top
	{ BlockVertex::PackPosition({ 1.0f, 1.0f, 1.0f }), BlockVertex_Normal4 | BlockVertex_TexCoord0 },
	{ BlockVertex::PackPosition({ 1.0f, 1.0f, 0.0f }), BlockVertex_Normal4 | BlockVertex_TexCoord2 },
	{ BlockVertex::PackPosition({ 0.0f, 1.0f, 0.0f }), BlockVertex_Normal4 | BlockVertex_TexCoord3 },
	{ BlockVertex::PackPosition({ 0.0f, 1.0f, 0.0f }), BlockVertex_Normal4 | BlockVertex_TexCoord3 },
	{ BlockVertex::PackPosition({ 0.0f, 1.0f, 1.0f }), BlockVertex_Normal4 | BlockVertex_TexCoord1 },
	{ BlockVertex::PackPosition({ 1.0f, 1.0f, 1.0f }), BlockVertex_Normal4 | BlockVertex_TexCoord0 },

	// Bottom
	{ BlockVertex::PackPosition({ 0.0f, 0.0f, 0.0f }), BlockVertex_Normal5 | BlockVertex_TexCoord3 },
	{ BlockVertex::PackPosition({ 1.0f, 0.0f, 0.0f }), BlockVertex_Normal5 | BlockVertex_TexCoord2 },
	{ BlockVertex::PackPosition({ 1.0f, 0.0f, 1.0f }), BlockVertex_Normal5 | BlockVertex_TexCoord0 },
	{ BlockVertex::PackPosition({ 1.0f, 0.0f, 1.0f }), BlockVertex_Normal5 | BlockVertex_TexCoord0 },
	{ BlockVertex::PackPosition({ 0.0f, 0.0f, 1.0f }), BlockVertex_Normal5 | BlockVertex_TexCoord1 },
	{ BlockVertex::PackPosition({ 0.0f, 0.0f, 0.0f }), BlockVertex_Normal5 | BlockVertex_TexCoord3 }
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
	{ GL_UNSIGNED_INT, 1 }, // a_Pos
	{ GL_UNSIGNED_INT, 1 }, // a_Data
} };

int Block::GetAxisCount(uint8_t id) {
	uint8_t actual_id = id & ~OrientationMask;
	if (HasProperty(actual_id, BlockProperty_HasOrientation3Axis ^ BlockProperty_HasOrientation)) return 3;
	if (HasProperty(actual_id, BlockProperty_HasOrientation4Axis ^ BlockProperty_HasOrientation)) return 4;
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

TextureIDs LoadTextureIDs(const JSON::Node& data_tree) {
	ASSERT(data_tree.type == JSON::NodeType::Array);
	int count = data_tree.Size();
	ASSERT(count == 1 || count == 3 || count == 6);
	
	if (count == 1) return TextureIDs(TEX_COORD(data_tree.children_array[0]));
	if (count == 3) return TextureIDs(TEX_COORD(data_tree.children_array[0]), TEX_COORD(data_tree.children_array[1]), TEX_COORD(data_tree.children_array[2]));
	if (count == 6) return TextureIDs(TEX_COORD(data_tree.children_array[0]), TEX_COORD(data_tree.children_array[1]), TEX_COORD(data_tree.children_array[2]), TEX_COORD(data_tree.children_array[3]), TEX_COORD(data_tree.children_array[4]), TEX_COORD(data_tree.children_array[5]));
	return TextureIDs();
}
uint16_t StringToProperty(const std::string& property_str) {
	if (property_str == "Transparent")      return BlockProperty_Transparent;
	if (property_str == "Glass")            return BlockProperty_Glass;
	if (property_str == "CrossMesh")        return BlockProperty_CrossMesh;
	if (property_str == "DisableCollision") return BlockProperty_DisableCollision;
	if (property_str == "Unbreakable")      return BlockProperty_Unbreakable;
	if (property_str == "3AxisOrientation") return BlockProperty_HasOrientation3Axis;
	if (property_str == "4AxisOrientation") return BlockProperty_HasOrientation4Axis;
	if (property_str == "LightEmitting")    return BlockProperty_LightEmitting;
	return 0;
}
void Block::LoadData(const std::string& filename) {
	JSON::Node data_tree = JSON::Parser::Parse(filename);
	
	s_MissingTexture = LoadTextureIDs(data_tree["MissingTexture"]);

	JSON::Node& break_progress_ids = data_tree["BreakProgressTextureCoords"];
	for (int i = 0; i < break_progress_ids.children_array.size(); i++) {
		s_BreakTextureIDs.push_back(TextureIDs(TEX_COORD(break_progress_ids.children_array[i])));
	}

	JSON::Node& block_data = data_tree["BlockData"];
	s_Data.resize(block_data.Size());
	for (int i = 0; i < block_data.Size(); i++) {
		JSON::Node& child = block_data.children_array[i];
		BlockData& data = s_Data[i];
		
		data.name = child["Name"];
		data.texture_ids = LoadTextureIDs(child["TextureIDs"]);
		if (child["BreakTime"].IsNull()) data.break_time = 0.0f;
		else data.break_time = child["BreakTime"];

		for (JSON::Node& property : child["Properties"].children_array) {
			data.properties |= StringToProperty(property);
		}
	}

}
const TextureIDs& Block::GetBreakTextureIDs(uint8_t state) {
	if (state >= s_BreakTextureIDs.size()) return s_MissingTexture;
	return s_BreakTextureIDs[state];
}
const TextureIDs& Block::GetTextureIDs(uint8_t id) {
	uint8_t real_id = id & ~OrientationMask;
	if (real_id >= BlockID_Count) return s_MissingTexture;
	return s_Data[real_id].texture_ids;
}
uint16_t Block::GetProperties(uint8_t id) {
	uint8_t real_id = id & ~OrientationMask;
	if (real_id >= BlockID_Count) return 0;
	return s_Data[real_id].properties;
}
bool Block::HasProperty(uint8_t id, uint16_t property) {
	uint8_t real_id = id & ~OrientationMask;
	if (real_id >= BlockID_Count) return false;
	return s_Data[real_id].properties & property;
}
const BlockData& Block::GetData(uint8_t id) {
	uint8_t real_id = id & ~OrientationMask;
	if (real_id >= BlockID_Count) return s_Data[0]; // return air
	return s_Data[real_id];
}

uint32_t BlockVertex::PackPosition(const glm::vec3& position) {
	uint32_t position_bits = 0;
	position_bits |= (uint8_t)position.x;
	position_bits |= (uint16_t)position.y << 8;
	position_bits |= (uint8_t)position.z << 24;
	return position_bits;
}
glm::vec3 BlockVertex::UnpackPosition(uint32_t position_bits) {
	glm::vec3 position;
	position.x = (float)(position_bits & 0x000000FF);
	position.y = (float)((position_bits & 0x00FFFF00) >> 8);
	position.z = (float)((position_bits & 0xFF000000) >> 24);
	return position;
}
