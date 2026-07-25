#include "Game/Inventory.h"
#include "Game/Game.h"
#include "Game/Voxel.h"
#include "Game/UI.h"

Item Item::Invalid = Item{ Item::InvalidID, 0 };
Inventory::Inventory(int width, int height)
	: m_Width(width), m_Height(height)
{
	m_Items.resize(m_Width * m_Height, { 0, 0 });
}
Item& Inventory::GetItem(const glm::ivec2& position) {
	if (position.x < 0 || position.x >= m_Width) return Item::Invalid;
	if (position.y < 0 || position.y >= m_Height) return Item::Invalid;
	size_t index = position.x + position.y * m_Width;
	return m_Items[index];
}

bool Item::IsBlock() {
	uint16_t bit_mask = 0xFF00;
	return (!(id & bit_mask) && id != 0);
}

void Item::ShowPreview(const glm::vec3& position, float scale) {
	if (IsBlock()) {
		uint8_t block_id = (uint8_t)id - 1;
		uint32_t texture_id = Block::BlockTextureIDs[block_id].front;
		UI::AtlasQuad(position, glm::vec2(scale), Game::GetTexture(TextureType::BlockAtlas), glm::vec2(16.0f), texture_id);
	}
}