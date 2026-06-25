#include "Core/ImGuiUtils.h"
#include "imgui.h"
#include "imgui_internal.h"

void ImGuiImage(const std::string& lable, const Ref<Texture>& texture, const glm::vec2& size, float column_width) {
	ImGui::BeginColumns(("##Columns_" + lable).c_str(), 2, ImGuiOldColumnFlags_NoResize);
	ImGui::SetColumnWidth(0, column_width);
	ImGui::SeparatorText(lable.c_str());
	ImGui::Text("Size: ( %d, %d )", texture->GetWidth(), texture->GetHeight());
	ImGui::Text("Slot: %d", texture->GetSlot());
	ImGui::NextColumn();
	
	ImVec2 avalible_space = ImGui::GetContentRegionAvail();
	float width  = avalible_space.x;
	float height = avalible_space.y;

	if (size.x != 0.0f) width = std::min(size.x, width);
	if (size.y != 0.0f) height = size.y;

	if (width < height) {
		height = (width * texture->GetHeight()) / texture->GetWidth();
	}
	else {
		width = (height * texture->GetWidth()) / texture->GetHeight();
	}
	
	ImGui::Image((ImTextureRef)texture->GetHandle(), ImVec2(width, height));

	ImGui::EndColumns();
}