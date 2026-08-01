#include "Game/Inventory.h"
#include "Game/Game.h"
#include "Game/Voxel.h"
#include "Game/UI.h"

#include <GLFW/glfw3.h>

Item Item::Invalid = Item{ Item::InvalidID, 0 };
Inventory::Inventory(int width, int height, float scale, float slot_size, float slot_padding, const glm::vec2& offset, const Ref<Texture>& ui_texture)
	: m_Width(width), m_Height(height), m_Scale(scale), m_UITexture(ui_texture)
{
	m_Items.resize(m_Width * m_Height, { 0, 0 });
	
	float tex_width = ui_texture->GetWidth();
	float tex_height = ui_texture->GetHeight();

	// transform from pixel space to texture space
	m_SlotSize = (slot_size / tex_height) * m_Scale;
	m_SlotPadding = (slot_padding / tex_height) * m_Scale;

	glm::vec2 padding = (offset / tex_height) * m_Scale * 2.0f;
	m_Offset = {
		-(tex_width / tex_height) * m_Scale + m_SlotSize + padding.x,
		-m_Scale + m_SlotSize + padding.y
	};
}
Item& Inventory::GetItem(const glm::ivec2& position) {
	if (position.x < 0 || position.x >= m_Width) return Item::Invalid;
	if (position.y < 0 || position.y >= m_Height) return Item::Invalid;
	size_t index = position.x + position.y * m_Width;
	return m_Items[index];
}

glm::ivec2 Inventory::GetHoveredSlot(const glm::vec2& mouse_pos) {
	float slot_advance = GetSlotAdvance();
	glm::vec2 area_min = {
		m_Offset.x - m_SlotSize,
		m_Offset.y - m_SlotSize
	};
	glm::vec2 area_max = {
		m_Offset.x - m_SlotSize + (m_Width * slot_advance),
		m_Offset.y - m_SlotSize + (m_Height * slot_advance)
	};
	area_min -= m_SlotPadding * 2.0f;
	area_max -= m_SlotPadding * 2.0f;
	if (mouse_pos.x < area_min.x || mouse_pos.x > area_max.x) return { -1, -1 };
	if (mouse_pos.y < area_min.y || mouse_pos.y > area_max.y) return { -1, -1 };

	glm::ivec2 slot = {
		(int)((mouse_pos.x - area_min.x) / slot_advance),
		(int)((mouse_pos.y - area_min.y) / slot_advance)
	};
	
	return slot;
}

void Inventory::RenderSlots() {
	// Render slots
	for (int y = 0; y < m_Height; y++) {
		for (int x = 0; x < m_Width; x++) {
			glm::vec3 slot_position = glm::vec3(m_Offset, -0.5f);
			slot_position.x += x * (m_SlotSize + m_SlotPadding) * 2.0f;
			slot_position.y += y * (m_SlotSize + m_SlotPadding) * 2.0f;
			GetItem({ x,y }).ShowPreview(slot_position, m_SlotSize * 0.6f);
		}
	}
}

bool Item::IsValid() {
	if (id == InvalidID) return false;
	if (count == 0) return false;
	return true;
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

	if (count > 1) {
		UI::Text(std::to_string(count), position + glm::vec3(scale * 1.79f, scale * 0.2f, 0.4f), glm::vec2(scale * 1.4f), TextAlignment_Right, glm::vec3(1.0f), glm::vec4(0.0f));
	}
}

InventoryHandler::InventoryHandler(const std::vector<Ref<Inventory>>& inventories)
	: m_Inventories(inventories) 
{ 
	m_HoveredSlot = glm::ivec2(-1);
	m_HoveredInventory = -1;

	m_HeldItem = Item::Invalid;
}
void InventoryHandler::OnLeftClick() {
	if (m_HoveredInventory == -1) return;

	Ref<Inventory>& inventory = m_Inventories[m_HoveredInventory];
	Item& slot_item = inventory->GetItem(m_HoveredSlot);
	if (slot_item.IsValid() && m_HeldItem.IsValid()) {
		// Swap items in hand and slot if already holding an item
		if (m_HeldItem.id != slot_item.id) {
			Item other = slot_item;
			slot_item = m_HeldItem;
			m_HeldItem = other;
		}
		// Combines stacks in slot if items are the same
		else {
			uint8_t item_count = m_HeldItem.count;
			if (slot_item.count + item_count > Item::StackSize) item_count = Item::StackSize - slot_item.count;

			slot_item.count += item_count;
			m_HeldItem.count -= item_count;
			if (m_HeldItem.count == 0) m_HeldItem = Item::Invalid;
		}
	}
	// Pick up stack
	else if (slot_item.IsValid() && !m_HeldItem.IsValid()) {
		m_HeldItem = slot_item;
		slot_item = Item::Invalid;
	}
	// Place stack
	else if (!slot_item.IsValid() && m_HeldItem.IsValid()) {
		slot_item = m_HeldItem;
		m_HeldItem = Item::Invalid;
	}
}
void InventoryHandler::OnRightClick() {
	if (m_HoveredInventory == -1) return;

	Ref<Inventory>& inventory = m_Inventories[m_HoveredInventory];
	Item& slot_item = inventory->GetItem(m_HoveredSlot);

	// Put down one item at a time
	if (m_HeldItem.IsValid() && (!slot_item.IsValid() || slot_item.id == m_HeldItem.id) && slot_item.count != Item::StackSize) {
		slot_item.id = m_HeldItem.id;
		slot_item.count += 1;
		m_HeldItem.count -= 1;
		if (m_HeldItem.count == 0) m_HeldItem = Item::Invalid;
	}
	// Pick up half a stack
	else if (slot_item.IsValid() && (!m_HeldItem.IsValid() || m_HeldItem.id == slot_item.id) && m_HeldItem.count != Item::StackSize) {
		m_HeldItem.id = slot_item.id;
		uint8_t item_count = ceil((float)slot_item.count / 2.0f);
		if (m_HeldItem.count + item_count > Item::StackSize) item_count = Item::StackSize - m_HeldItem.count;
		m_HeldItem.count += item_count;
		slot_item.count -= item_count;
		if (slot_item.count == 0) slot_item = Item::Invalid;
	}
}
void InventoryHandler::Render() {
	// Get mouse world position
	double mouse_x = 0.0, mouse_y = 0.0;
	glfwGetCursorPos((GLFWwindow*)Game::GetWindow()->GetHandle(), &mouse_x, &mouse_y);
	glm::vec2 mouse_pos = UI::GetWorldPosition({ mouse_x, mouse_y });

	m_HoveredInventory = -1;

	for (int i = 0; i < m_Inventories.size(); i++) {
		Ref<Inventory>& inventory = m_Inventories[i];
		inventory->RenderSlots();

		// Check if mouse is hovering current inventory
		glm::ivec2 hovered_slot = inventory->GetHoveredSlot(mouse_pos);
		if (hovered_slot != glm::ivec2(-1)) {
			m_HoveredSlot = hovered_slot;
			m_HoveredInventory = i;

			// Highlight hovered slot
			float slot_advance = inventory->GetSlotAdvance();
			glm::vec3 slot_highlight_pos = {
				inventory->GetOffset().x + slot_advance * hovered_slot.x,
				inventory->GetOffset().y + slot_advance * hovered_slot.y,
				0.0f
			};
			UI::ColoredQuad(slot_highlight_pos, glm::vec2(inventory->GetSlotSize()), glm::vec4(1.0f, 1.0f, 1.0f, 0.65f));
		}
	}

	// Render held item
	m_HeldItem.ShowPreview(glm::vec3(mouse_pos, 0), m_Inventories[0]->GetSlotSize() * 0.6f);
}
