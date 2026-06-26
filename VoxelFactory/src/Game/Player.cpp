#include "Game/Player.h"
#include "Game/Game.h"
#include "Core/Utils.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <imgui.h>

#include <iostream>

Player::Player(const glm::vec3& position, const Ref<Camera>& camera, const Ref<World>& world)
	: m_Position(position), m_Camera(camera), m_World(world) {

	m_CameraOffset = glm::vec3(0.0f, 1.7f, 0.0f);
	m_Camera->position = m_Position + m_CameraOffset;
	m_Camera->UpdateView();

	glm::vec3 collider_size = glm::vec3(0.8f, 1.8f, 0.8f);
	m_ColliderOffset = glm::vec3(0.0f, collider_size.y * 0.5f, 0.0f);
	m_Collider = AABB{ m_Position + m_ColliderOffset, collider_size };
	m_Velocity = glm::vec3(0.0f);
	m_GravitationalConstant = -0.388f;
	m_Drag = 0.901f;

	m_GroundCheckDist = 0.05f;
	m_OnGround = false;

	m_SelectorPosition = glm::vec3(0.0f);
	m_ShowSelector = false;

	m_Flight = false;
	m_MouseSensitivity = 0.1f;
	m_WalkSpeed = 5.612f;
	m_Speed = m_WalkSpeed;
	m_SprintMultiplier = 1.5f;
	m_JumpForce = 0.125f;
	m_Reach = 7.0f;

	InitUI();
	InitSelector();
}

void Player::ShowImGui() {
	// Position stuff
	glm::vec3 block_pos = Chunk::GetBlockPosition(m_Position);
	glm::vec3 chunk_pos = Chunk::GetBlockChunkPosition(m_Position);
	glm::vec3 chunk_block_pos = Chunk::GetBlockLocalPosition(block_pos);

	ImGui::SeparatorText("General");
	if (ImGui::InputFloat3("Position", glm::value_ptr(m_Position))) {
		m_Collider.SetPosition(m_Position);
		m_Camera->position = m_Position + m_CameraOffset;
		m_Camera->UpdateView();
	}
	if (ImGui::DragFloat3("Camera Offset", glm::value_ptr(m_CameraOffset), 0.1f)) {
		m_Camera->position = m_Position + m_CameraOffset;
		m_Camera->UpdateView();
	}
	ImGui::Text("Block Position: ( %d, %d, %d )", (int)block_pos.x, (int)block_pos.y, (int)block_pos.z);
	ImGui::Text("Chunk Block Position: ( %d, %d, %d )", (int)chunk_block_pos.x, (int)chunk_block_pos.y, (int)chunk_block_pos.z);
	ImGui::Text("Chunk Position: ( %d, %d )", (int)chunk_pos.x, (int)chunk_pos.z);

	// Physics
	ImGui::SeparatorText("Physics");
	ImGui::Text("Velocity: %s", VEC3_STR(m_Velocity).c_str());
	if (ImGui::DragFloat3("Collider Size", glm::value_ptr(m_Collider.size), 0.1f)) {
		m_Collider.CalculateMinMax();
	}
	ImGui::Text("Collider Position: %s", VEC3_STR(m_Collider.position).c_str());
	ImGui::Text("Collider Min: %s", VEC3_STR(m_Collider.min).c_str());
	ImGui::Text("Collider Max: %s", VEC3_STR(m_Collider.max).c_str());
	ImGui::Text("On Ground: %s", (m_OnGround ? "true" : "false"));
	ImGui::DragFloat("Ground Check Distance", &m_GroundCheckDist);
	ImGui::DragFloat("Gravity", &m_GravitationalConstant, 0.1f);
	ImGui::DragFloat("Drag", &m_Drag, 0.1f);

	// Selector
	ImGui::SeparatorText("Selector");
	ImGui::Text("Visible: %s", (m_ShowSelector ? "true" : "false"));
	ImGui::Text("Position: ( %d, %d, %d )", (int)m_SelectorPosition.x, (int)m_SelectorPosition.y, (int)m_SelectorPosition.z);

	// Input
	ImGui::SeparatorText("Input");
	ImGui::Checkbox("Flight", &m_Flight);
	ImGui::DragFloat("Walk Speed", &m_WalkSpeed, 0.1f);
	ImGui::DragFloat("Sprint Multiplier", &m_SprintMultiplier, 0.1f);
	ImGui::Text("Speed: %f", m_Speed);
	ImGui::DragFloat("Jump Force", &m_JumpForce);
	ImGui::DragFloat("Mouse Sensitivity", &m_MouseSensitivity);
	ImGui::DragFloat("Reach", &m_Reach);
}

void Player::Update(float delta_time, glm::vec2& last_mouse_pos, const Ref<Window>& window) {

	Input(delta_time, window);

	// Check for/resolve collisions
	glm::vec3 collision_normal = glm::vec3(0.0f);
	m_Collider.CalculateMinMax();
	m_World->ResolveDynamicAABB(m_Collider, m_Velocity, collision_normal);
	m_Collider.position += m_Velocity;
	m_Position = m_Collider.position - m_ColliderOffset;

	// Create hitbox directly beneath the player
	AABB ground_check;
	m_Collider.CalculateMinMax();
	ground_check.max = glm::vec3(m_Collider.max.x, m_Collider.min.y, m_Collider.max.z);
	ground_check.min = m_Collider.min - glm::vec3(0.0f, m_GroundCheckDist, 0.0f);
	ground_check.position = (ground_check.min + ground_check.max) * 0.5f;
	ground_check.size = ground_check.max - ground_check.min;

	// Check if player is on the ground
	m_OnGround = !m_World->AABBIntersectedVoxels(ground_check).empty();

	CameraInput(last_mouse_pos, window);

	// Selector ray
	CollisionResultData ray_result = m_World->CastRay({ m_Camera->position, m_Camera->direction });
	m_ShowSelector = (ray_result.hit && ray_result.dist <= m_Reach);
	m_SelectorPosition = ray_result.voxel_position;
}
void Player::Render() {
	RenderSelector();
}

void Player::OnLeftClick() {
	if (!m_ShowSelector) return;

	// Delete voxel
	m_World->SetVoxel(m_SelectorPosition, 0);
}
void Player::OnRightClick() {
	if (!m_ShowSelector) return;
	CollisionResultData ray_data = m_World->CastRay({ m_Camera->position, m_Camera->direction });

	// Place dirt block
	glm::vec3 voxel = ray_data.voxel_position + ray_data.normal;
	if (m_World->WillIntersect(m_Collider, voxel)) return;
	m_World->SetVoxel(voxel, 3);
}

void Player::Input(float delta_time, const Ref<Window>& window) {
	GLFWwindow* window_handle = (GLFWwindow*)window->GetHandle();

	// Handle Sprint
	if (glfwGetKey(window_handle, GLFW_KEY_LEFT_SHIFT)) m_Speed = m_WalkSpeed * m_SprintMultiplier;
	else m_Speed = m_WalkSpeed;

	// Direction vectors
	glm::vec3 up_dir = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 right_dir = glm::normalize(glm::cross(m_Camera->direction, up_dir));
	glm::vec3 forward_dir = glm::normalize(glm::cross(up_dir, right_dir));

	m_Velocity.x = 0.0f;
	m_Velocity.z = 0.0f;
	if (m_Flight) m_Velocity.y = 0.0f;
	if (glfwGetKey(window_handle, GLFW_KEY_W)) m_Velocity += forward_dir * m_Speed * delta_time;
	if (glfwGetKey(window_handle, GLFW_KEY_S)) m_Velocity -= forward_dir * m_Speed * delta_time;
	if (glfwGetKey(window_handle, GLFW_KEY_A)) m_Velocity -= right_dir * m_Speed * delta_time;
	if (glfwGetKey(window_handle, GLFW_KEY_D)) m_Velocity += right_dir * m_Speed * delta_time;

	if (glfwGetKey(window_handle, GLFW_KEY_SPACE) && m_Flight) m_Velocity += up_dir * m_Speed * delta_time;
	if (glfwGetKey(window_handle, GLFW_KEY_LEFT_CONTROL) && m_Flight) m_Velocity -= up_dir * m_Speed * delta_time;

	// Apply Gravity
	if (!m_Flight) m_Velocity.y += m_GravitationalConstant * delta_time;

	// Apply drag
	float drag_force = m_Velocity.y * m_Drag * delta_time;
	if (m_OnGround) m_Velocity.y -= drag_force;
	else m_Velocity -= drag_force;

	// Handle Jump
	if (glfwGetKey(window_handle, GLFW_KEY_SPACE) && m_OnGround) {
		m_Velocity.y = m_JumpForce;
	}
}
void Player::CameraInput(glm::vec2& last_mouse_pos, const Ref<Window>& window) {
	GLFWwindow* window_handle = (GLFWwindow*)window->GetHandle();
	
	// Get mouse delta
	double mouse_x = 0.0, mouse_y = 0.0;
	glfwGetCursorPos(window_handle, &mouse_x, &mouse_y);
	glm::vec2 mouse_pos = glm::vec2(static_cast<float>(mouse_x), static_cast<float>(mouse_y));
	glm::vec2 delta_mouse = {
		mouse_pos.x - last_mouse_pos.x,
		last_mouse_pos.y - mouse_pos.y // Invert Y, because opengl is bottom to top
	};
	delta_mouse *= ((m_MouseSensitivity * PI) / 180.0f);

	last_mouse_pos = mouse_pos;

	// Adjust rotation
	m_Camera->eular.y += delta_mouse.x; // Yaw
	m_Camera->eular.x += delta_mouse.y; // Pitch

	// Cap pitch
	constexpr float c_MaxPitch = PIHalf - (PI / 180.0f); // 89 degrees
	constexpr float c_MinPitch = -PIHalf + (PI / 180.0f); // -89 degrees
	m_Camera->eular.x = std::min(c_MaxPitch, std::max(m_Camera->eular.x, c_MinPitch));

	if (delta_mouse != glm::vec2(0.0f)) {
		m_Camera->direction = Camera::EulerDirection(m_Camera->eular.x, m_Camera->eular.y);
	}

	m_Camera->position = m_Position + m_CameraOffset;
	m_Camera->UpdateView();
}

void Player::InitUI() {

	m_CrossHair = CreateRef<Texture>("assets/textures/CrossHair.png");

}
void Player::RenderUI(Ref<Shader>& ui_shader)
{
	glm::mat4 cross_hair_model = glm::mat4(1.0f);
	ui_shader->SetUniform("u_Model", cross_hair_model);
	ui_shader->SetUniform("u_Tint", glm::vec3(1.0f));
	
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}

void Player::InitSelector() {
	m_SelectorMesh = CreateRef<VertexArray>();
	m_SelectorMesh->Bind();

	VertexLayout vertex_layout = { {
		{ GL_FLOAT, 3 },
		{ GL_FLOAT, 3 }
	} };
	Ref<VertexBuffer> vertex_buffer = CreateRef<VertexBuffer>(c_SelectorVertices, 48 * sizeof(float), vertex_layout);
	m_SelectorMesh->GetVertexBuffer() = vertex_buffer;

	Ref<IndexBuffer> index_buffer = CreateRef<IndexBuffer>(c_SelectorIndices, 24 * sizeof(uint32_t));
	m_SelectorMesh->GetIndexBuffer() = index_buffer;

	m_SelectorMesh->Unbind();

	m_SelectorShader = CreateRef<Shader>("assets/shaders/Selector.vert", "assets/shaders/Selector.frag");
}
void Player::RenderSelector() {
	if (!m_ShowSelector) return;
	m_SelectorShader->Bind();
	m_SelectorShader->SetUniform("u_ViewProjection", m_Camera->view_projection);

	glm::mat4 model = glm::translate(glm::mat4(1.0f), m_SelectorPosition + glm::vec3(0.5f));
	m_SelectorShader->SetUniform("u_Model", model);

	m_SelectorMesh->Bind();
	glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, nullptr);
	m_SelectorMesh->Unbind();

	m_SelectorShader->Unbind();
}