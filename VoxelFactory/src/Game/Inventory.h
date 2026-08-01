#pragma once
#include <glm/glm.hpp>
#include "Core/Core.h"
#include "Renderer/Texture.h"

struct Item {
	uint16_t id;
	uint8_t count;

	bool IsValid();
	bool IsBlock();
	void ShowPreview(const glm::vec3& position, float scale);

	inline static constexpr uint16_t InvalidID = std::numeric_limits<uint16_t>::max();
	static Item Invalid;
	const inline static uint8_t StackSize = 64;
};

class Inventory {
public:
	Inventory(int width, int height, float scale, float slot_size, float slot_padding, const glm::vec2& offset, const Ref<Texture>& ui_texture);
	~Inventory() {}

	Item& GetItem(const glm::ivec2& position);
	glm::ivec2 GetHoveredSlot(const glm::vec2& mouse_pos);

	float GetScale() { return m_Scale; }
	float GetSlotSize() { return m_SlotSize; }
	float GetSlotPadding() { return m_SlotPadding; }
	float GetSlotAdvance() { return (m_SlotSize + m_SlotPadding) * 2.0f; }
	const glm::vec2& GetOffset() { return m_Offset; }

	void RenderSlots();

private:
	int m_Width;
	int m_Height;

	std::vector<Item> m_Items;

	// UI vars
	float m_Scale; // Half the height of UI
	float m_SlotSize;
	float m_SlotPadding;
	glm::vec2 m_Offset;
	Ref<Texture> m_UITexture;
};

class InventoryHandler {
public:
	InventoryHandler(const std::vector<Ref<Inventory>>& inventories);
	~InventoryHandler() { }

	void OnLeftClick();
	void OnRightClick();

	void Render();

private:
	std::vector<Ref<Inventory>> m_Inventories;

	int m_HoveredInventory;
	glm::ivec2 m_HoveredSlot;

	Item m_HeldItem;
};