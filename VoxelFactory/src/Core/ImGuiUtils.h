#pragma once
#include "Core/Core.h"
#include "Renderer/Texture.h"
#include <glm/glm.hpp>

void ImGuiImage(const std::string& lable, const Ref<Texture>& texture, const glm::vec2& size = glm::vec2(0.0f), float column_width = 150.0f);