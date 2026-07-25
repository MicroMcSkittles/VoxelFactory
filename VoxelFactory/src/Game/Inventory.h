#pragma once
#include <glm/glm.hpp>
#include "Core/Core.h"
#include "Renderer/Texture.h"

struct Item {
	uint16_t id;
	uint8_t count;

	bool IsBlock();
	void ShowPreview(const glm::vec3& position, float scale);

	inline static constexpr uint16_t InvalidID = std::numeric_limits<uint16_t>::max();
	static Item Invalid;
};

class Inventory {
public:
	Inventory(int width, int height);
	~Inventory() {}

	Item& GetItem(const glm::ivec2& position);

private:
	int m_Width;
	int m_Height;

	std::vector<Item> m_Items;
};