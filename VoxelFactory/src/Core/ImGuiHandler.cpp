#include "Core/ImGuiHandler.h"
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

void ImGuiHandler::Init(const Ref<Window>& window) {
	GLFWwindow* window_handle = reinterpret_cast<GLFWwindow*>(window->GetHandle());
	
	ImGui::CreateContext();

	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window_handle, true);
	ImGui_ImplOpenGL3_Init("#version 460");
}

void ImGuiHandler::StartFrame() {
	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();
}
void ImGuiHandler::EndFrame() {
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}