#include "Game/Player.h"
#include "Game/Game.h"
#include "Game/UI/UI.h"
#include "Core/Utils.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <imgui.h>

#include <iostream>
#include <sstream>
#include <iomanip>

Hotbar::Hotbar() {
	selected = 0;
	inventory = CreateRef<Inventory>(Width, 1, 6.0f, 16.0f, 2.0f, glm::vec2(8, 8), Game::GetTexture(TextureType::Inventory));
}
Item& Hotbar::GetSelected() {
	return inventory->GetItem({ selected, 0 });
}
Item& Hotbar::Get(int index) {
	return inventory->GetItem({ index, 0 });
}

Player::Player(const glm::vec3& position, const Ref<Camera>& camera, const Ref<World>& world)
	: m_Position(position), m_Camera(camera), m_World(world) {

	// Fill inventory
	m_Inventory = CreateRef<Inventory>(9, 3, 6.0f, 16.0f, 2.0f, glm::vec2(8, 30), Game::GetTexture(TextureType::Inventory));
	for (int i = 1; i < std::min((int)BlockID_Count, 9 * 3); i++) {
		int x = (i - 1) % 9;
		int y = (i - 1) / 9;
		m_Inventory->GetItem({ x, y }).id = i;
		m_Inventory->GetItem({ x, y }).count = Item::StackSize;
	}
	m_InventoryHandler = nullptr;
	m_Hotbar.inventory->GetItem({ 0,0 }).id = ItemID_Axe;
	m_Hotbar.inventory->GetItem({ 0,0 }).count = 1;
	m_Hotbar.inventory->GetItem({ 1,0 }).id = ItemID_Pickaxe;
	m_Hotbar.inventory->GetItem({ 1,0 }).count = 1;
	m_Hotbar.inventory->GetItem({ 2,0 }).id = ItemID_Shovel;
	m_Hotbar.inventory->GetItem({ 2,0 }).count = 1;
	m_Hotbar.inventory->GetItem({ 3,0 }).id = ItemID_IronIngot;
	m_Hotbar.inventory->GetItem({ 3,0 }).count = 64;

	// Camera
	m_CameraOffset = glm::vec3(0.0f, 1.7f, 0.0f);
	m_Camera->position = m_Position + m_CameraOffset;
	m_Camera->UpdateView();
	m_Hand.direction = m_Camera->direction;

	// Physics
	glm::vec3 collider_size = glm::vec3(0.8f, 1.8f, 0.8f);
	m_ColliderOffset = glm::vec3(0.0f, collider_size.y * 0.5f, 0.0f);
	m_Collider = AABB{ m_Position + m_ColliderOffset, collider_size };
	m_Velocity = glm::vec3(0.0f);
	m_GravitationalConstant = -0.388f;
	m_Drag = 0.901f;

	m_GroundCheckDist = 0.05f;
	m_OnGround = false;

	// Selector
	m_SelectorPosition = glm::vec3(0.0f);
	m_SelectorNormal = glm::vec3(0.0f);
	m_ShowSelector = false;

	// Controls
	m_Flight = false;
	m_NoClip = false;
	m_OpenInventory = false;
	m_EnableCollision = true;
	m_InstantBlockBreaking = false;
	m_MouseSensitivity = 0.1f;
	m_WalkSpeed = 5.612f;
	m_Speed = m_WalkSpeed;
	m_SprintMultiplier = 1.5f;
	m_JumpForce = 0.125f;
	m_Reach = 7.0f;
	m_MovementTime = 0.0f;
	m_ItemDropForce = 0.125f;
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
	ImGui::Checkbox("Enable Collision", &m_EnableCollision); 
	ImGui::Checkbox("Instant Block Breaking", &m_InstantBlockBreaking);
	ImGui::DragFloat("Walk Speed", &m_WalkSpeed, 0.1f);
	ImGui::DragFloat("Sprint Multiplier", &m_SprintMultiplier, 0.1f);
	ImGui::Text("Speed: %f", m_Speed);
	ImGui::DragFloat("Jump Force", &m_JumpForce);
	ImGui::DragFloat("Item Drop Force", &m_ItemDropForce);
	ImGui::DragFloat("Mouse Sensitivity", &m_MouseSensitivity);
	ImGui::DragFloat("Reach", &m_Reach);
	ImGui::Text("Movement Time: %f", m_MovementTime);

	// Hand
	ImGui::SeparatorText("Hand");
	ImGui::Text("Current Animation: %s", m_Hand.animator.GetCurrentAnimationName().c_str());
	ImGui::Text("Current Animation Frame: %d", m_Hand.animator.GetCurrentFrameIndex());
	ImGui::Text("Current Animation Frame Time: %.2f", m_Hand.animator.GetCurrentFrameTime());
	ImGui::Text("Hand Position: %s", VEC3_STR(m_Hand.transform.position).c_str());
	ImGui::DragFloat3("Hand Position", &m_Hand.transform.position.x, 0.005f);
	ImGui::DragFloat3("Hand Rotation", &m_Hand.transform.rotation.x, 0.005f);
	ImGui::DragFloat3("Hand Scale", &m_Hand.transform.scale.x, 0.005f);
	ImGui::Text("Hand Item ID: %u", m_Hotbar.GetSelected().id);

	// Hotbar
	ImGui::SeparatorText("Hotbar");
	int hotbar_selected_index = m_Hotbar.selected;
	if (ImGui::InputInt("Hotbar Selected Index", &hotbar_selected_index)) {
		if (hotbar_selected_index < 0) hotbar_selected_index = Hotbar::Width - 1;
		else if (hotbar_selected_index >= Hotbar::Width) hotbar_selected_index = 0;
		else m_Hotbar.selected = hotbar_selected_index;
	}
	for (int i = 0; i < Hotbar::Width; i++) {
		int item[2] = { m_Hotbar.Get(i).id, m_Hotbar.Get(i).count };
		if (ImGui::InputInt2(std::to_string(i).c_str(), item)) {
			m_Hotbar.Get(i).id = item[0];
			m_Hotbar.Get(i).count = item[1];
		}
	}
}

void Player::Update(float delta_time, glm::vec2& last_mouse_pos) {

	CameraInput(last_mouse_pos);

	// Selector ray
	CollisionResultData ray_result = m_World->CastRay({ m_Camera->position, m_Camera->direction });
	m_ShowSelector = (ray_result.hit && ray_result.dist <= m_Reach);
	m_SelectorPosition = ray_result.voxel_position;
	m_SelectorNormal = ray_result.normal;
}
void Player::PhysicsUpdate(float delta_time) {
	Input(delta_time);
	HandInput(delta_time);

	// Check for/resolve collisions
	if (m_EnableCollision) {
		glm::vec3 collision_normal = glm::vec3(0.0f);
		m_Collider.CalculateMinMax();
		m_World->ResolveDynamicAABB(m_Collider, m_Velocity, collision_normal);
	}
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
	m_OnGround = !m_World->AABBIntersectedVoxels(ground_check).empty() && m_EnableCollision;

	// Update hand
	UpdateHand(delta_time);
}
void Player::Render() {
	RenderHand();
	RenderBlockOverlay();
	RenderSelector();
}

bool Player::OnKey(int key, int action, int mods) {
	if (key == GLFW_KEY_E && action == GLFW_PRESS) {
		if (m_OpenInventory) CloseInventory();
		else OpenInventory();
		return true;
	}
	else if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS && m_OpenInventory) {
		CloseInventory();
		return true;
	}
	else if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
		DropItem();
		return true;
	}
	else if (key == GLFW_KEY_N && mods & GLFW_MOD_SHIFT && action == GLFW_PRESS) {
		if (m_NoClip) {
			m_NoClip = false;
			m_Flight = false;
			m_EnableCollision = true;
			m_WalkSpeed /= 2.5f;
		}
		else {
			m_NoClip = true;
			m_Flight = true;
			m_EnableCollision = false;
			m_WalkSpeed *= 2.5f;
		}
	}

	return false;
}
void Player::OnLeftClick() {
	if (Game::GetState() == GameState::Menu && m_OpenInventory) m_InventoryHandler->OnLeftClick();
	if (Game::GetState() != GameState::InGame) return;
	if (!m_ShowSelector) {
		m_Hand.Swing();
		return;
	}
	
	if (m_InstantBlockBreaking) m_World->BreakVoxel(m_SelectorPosition);
	else {
		m_Hand.breaking = true;
		m_Hand.break_position = m_SelectorPosition;
	}
	m_Hand.Hit();
}
void Player::OnRightClick() {
	if (Game::GetState() == GameState::Menu && m_OpenInventory) m_InventoryHandler->OnRightClick();
	if (Game::GetState() != GameState::InGame) return;
	if (!m_ShowSelector) return;
	if (!m_Hotbar.GetSelected().IsBlock()) return;

	CollisionResultData ray_data = m_World->CastRay({ m_Camera->position, m_Camera->direction });

	// Find block oriantation
	uint8_t block_id = (uint8_t)m_Hotbar.GetSelected().id;
	if (Block::HasProperty(block_id, BlockProperty_HasOrientation)) {
		glm::vec3 direction = ray_data.normal;
		if (Block::GetAxisCount(block_id) == 4) direction = -(m_Camera->direction * ray_data.dist);
		block_id |= Block::CalculateOrientation(direction, block_id);
	}

	// Place block from players hand
	glm::vec3 voxel = ray_data.voxel_position + ray_data.normal;
	if (m_World->WillIntersect(m_Collider, voxel)) return;
	m_World->SetVoxel(voxel, block_id);
	
	Item& selected = m_Hotbar.GetSelected();
	selected.count -= 1;
	if (selected.count == 0) selected = Item::Invalid;

	m_Hand.Place();
}
void Player::OnScroll(float delta) {
	Item last_item = m_Hotbar.GetSelected();

	if (delta < 0.0f) m_Hotbar.selected += 1;
	else m_Hotbar.selected -= 1;

	if (m_Hotbar.selected < 0) m_Hotbar.selected = Hotbar::Width - 1;
	else if (m_Hotbar.selected >= Hotbar::Width) m_Hotbar.selected = 0;
	
	Item current_item = m_Hotbar.GetSelected();
	if (last_item.id != current_item.id) {
		m_Hand.animator.Play("SwapItems");
	}
}

void Player::OpenInventory(const Ref<Inventory>& other) {
	m_OpenInventory = true;
	Game::Get()->ReleaseMouse();
	Game::GetState() = GameState::Menu;

	std::vector<Ref<Inventory>> inventories = {
		m_Hotbar.inventory, m_Inventory, other
	};
	m_InventoryHandler = CreateRef<InventoryHandler>(inventories);
}
void Player::OpenInventory() {
	m_OpenInventory = true;
	Game::Get()->ReleaseMouse();
	Game::GetState() = GameState::Menu;

	std::vector<Ref<Inventory>> inventories = {
		m_Hotbar.inventory, m_Inventory
	};
	m_InventoryHandler = CreateRef<InventoryHandler>(inventories);
}
void Player::CloseInventory() {
	m_OpenInventory = false;
	m_InventoryHandler = nullptr;
	Game::Get()->CaptureMouse();
	Game::GetState() = GameState::InGame;
}

void Player::PushInventoryItems(Item& item) {
	m_Hotbar.inventory->PushItems(item);
	if (item.count == 0) return;
	m_Inventory->PushItems(item);
}
bool Player::HasItemSpace(const Item& item) {
	if (m_Hotbar.inventory->HasItemSpace(item)) return true;
	if (m_Inventory->HasItemSpace(item)) return true;
	return false;
}

void Player::Input(float delta_time) {
	GLFWwindow* window_handle = (GLFWwindow*)Game::GetWindow()->GetHandle();

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

	if ((m_Velocity.x != 0.0f || m_Velocity.z != 0.0f) && m_OnGround) {
		if (!m_Hand.animator.Playing()) m_Hand.animator.Play("Sway", m_Speed);
		else if (m_Hand.animator.IsPlaying("Sway")) m_Hand.animator.SetSpeed(m_Speed);
	}
	else {
		if (m_Hand.animator.IsPlaying("Sway")) m_Hand.animator.Stop();
	}

	if (glfwGetKey(window_handle, GLFW_KEY_SPACE) && m_Flight) m_Velocity += up_dir * m_Speed * delta_time;
	if (glfwGetKey(window_handle, GLFW_KEY_LEFT_CONTROL) && m_Flight) m_Velocity -= up_dir * m_Speed * delta_time;

	bool can_move = !m_OpenInventory && Game::IsMouseCaptured();
	if (!can_move) {
		m_Velocity.x = 0.0f;
		m_Velocity.z = 0.0f;
		m_MovementTime = 0.0f;
	}

	// Apply Gravity
	if (!m_Flight) m_Velocity.y += m_GravitationalConstant * delta_time;

	// Apply drag
	if (m_OnGround) m_Velocity.y -= m_Velocity.y * m_Drag * delta_time;
	else m_Velocity -= m_Velocity * m_Drag * delta_time;

	// Handle Jump
	if (glfwGetKey(window_handle, GLFW_KEY_SPACE) && m_OnGround && can_move) {
		m_Velocity.y = m_JumpForce;
	}
}
void Player::HandInput(float delta_time) {
	if (!m_Hand.breaking) return;

	GLFWwindow* window_handle = (GLFWwindow*)Game::GetWindow()->GetHandle();

	bool left_mouse = (glfwGetMouseButton(window_handle, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
	if (left_mouse && m_Hand.break_position == m_SelectorPosition && m_ShowSelector) {
		m_Hand.break_timer += delta_time;
		m_Hand.partical_timer -= delta_time;
	}
	else if (left_mouse && m_Hand.break_position != m_SelectorPosition && m_ShowSelector) {
		m_Hand.break_timer = 0.0f;
		m_Hand.partical_timer = 0.0f;
		m_Hand.break_position = m_SelectorPosition;
		m_Hand.break_frame = 0;
	}
	else {
		m_Hand.breaking = false;
		m_Hand.break_timer = 0.0f;
		m_Hand.partical_timer = 0.0f;
		m_Hand.break_frame = 0;
		m_Hand.animator.Stop();
		return;
	}

	if (m_Hand.partical_timer <= 0.0f) {
		m_Hand.partical_timer = m_Hand.partical_rate;
		m_World->SpawnSurfaceParticals(m_Hand.break_position, m_SelectorNormal, 6, *(uint32_t*)&m_Hand.break_timer);
	}

	uint8_t block_id = m_World->GetVoxel(m_Hand.break_position).id;
	if (Block::HasProperty(block_id, BlockProperty_Unbreakable)) return;

	float block_break_time = Block::GetData(block_id).break_time;
	m_Hand.break_frame = std::min((int)(10.0f * (m_Hand.break_timer / block_break_time)), 9);

	if (m_Hand.break_timer >= block_break_time) {
		m_Hand.break_timer = 0.0f;
		m_Hand.break_frame = 0;
		m_Hand.partical_timer = 0.0f;

		m_World->BreakVoxel(m_SelectorPosition);
	}
}
void Player::CameraInput(glm::vec2& last_mouse_pos) {
	if (m_OpenInventory || !Game::IsMouseCaptured()) {
		m_Camera->position = m_Position + m_CameraOffset;
		m_Camera->UpdateView();
		return;
	}

	GLFWwindow* window_handle = (GLFWwindow*)Game::GetWindow()->GetHandle();
	
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

void Player::RenderUI() {
	if (m_OpenInventory) {
		RenderInventoryUI();
		return;
	}

	// Cross Hair
	UI::TexturedQuad(glm::vec3(0.0f), glm::vec2(0.25f), Game::GetTexture(TextureType::CrossHair));

	glm::vec2 screen_min = UI::GetScreenMin();
	Ref<Texture>& hotbar_texture = Game::GetTexture(TextureType::Hotbar);
	
	float hotbar_scale = 0.75f;
	float hotbar_width = hotbar_texture->GetWidth();
	float hotbar_height = hotbar_texture->GetHeight();

	float slot_width = (32.0f / hotbar_height) * hotbar_scale;
	float slot_padding = (8.0f / hotbar_height) * hotbar_scale * 2.0f;
	float hotbar_padding = (6.0f / hotbar_height) * hotbar_scale * 2.0f;
	glm::vec2 slot_offset = {
		-(hotbar_width / hotbar_height) * hotbar_scale + slot_width + hotbar_padding,
		screen_min.y + hotbar_scale
	};

	// Hotbar
	UI::TexturedQuad(glm::vec3(0.0f, screen_min.y + hotbar_scale, -0.5f), hotbar_scale, hotbar_texture, true);

	// Item previews
	for (int i = 0; i < Hotbar::Width; i++) {
		glm::vec3 preview_position = {
			slot_offset.x + ((slot_width * 2.0f + slot_padding) * i),
			slot_offset.y,
			-0.25f
		};
		m_Hotbar.Get(i).ShowPreview(preview_position, hotbar_scale * 0.5f);
	}

	// Hotbar selector
	Ref<Texture>& selector_texture = Game::GetTexture(TextureType::HotbarSelector);
	glm::vec3 selector_position = {
		slot_offset.x + ((slot_width * 2.0f + slot_padding) * m_Hotbar.selected),
		slot_offset.y,
		0.0f
	};
	float selector_scale = (selector_texture->GetHeight() / hotbar_height) * hotbar_scale;
	UI::TexturedQuad(selector_position, selector_scale, selector_texture, true);
}
void Player::RenderInventoryUI() {
	glm::vec2 screen_min = UI::GetScreenMin();
	glm::vec2 screen_max = UI::GetScreenMax();

	// Tint screen
	glm::vec2 screen_tint_size = (screen_max - screen_min) * 0.5f;
	UI::ColoredQuad(glm::vec3(0.0f, 0.0f, -1.0f), screen_tint_size, UI::GetColor(ColorType::ScreenTint));
	
	// Show inventory
	UI::TexturedQuad({ 0.0f, 0.0f, -0.9f }, 6.0f, Game::GetTexture(TextureType::Inventory));
	m_InventoryHandler->Render();
}

std::string Player::StatsText() {

	std::stringstream ss;

	glm::vec3 local_position = Chunk::GetBlockLocalPosition(m_Position);
	glm::vec3 chunk_position = Chunk::GetBlockChunkPosition(m_Position);
	ss << "Pos " << VEC3_STR(m_Position) << "\n";
	ss << "Local " << VEC3_STR(local_position) << "\n";
	ss << "Chunk " << VEC3_STR(chunk_position) << "\n";
	ss << "Speed " << m_Speed << "\n";
	ss << "Velocity " << VEC3_STR(m_Velocity) << "\n";
	ss << "Hand Direction " << VEC3_STR(m_Hand.direction) << "\n";
	ss << "Camera Direction " << VEC3_STR(m_Camera->direction) << "\n";
	ss << "Break Timer " << std::format("{:.2f}", m_Hand.break_timer) << "\n";

	if (m_ShowSelector) {
		ss << "Looking at " << VEC3_STR(m_SelectorPosition) << "\n";
	}

	return ss.str();
}

void Player::RenderBlockOverlay() {
	if (!m_Hand.breaking) return;

	Ref<Shader>& overlay_shader = Game::GetShader(ShaderType::BlockPreview);
	overlay_shader->Bind();

	Ref<Texture>& atlas = Game::GetTexture(TextureType::BlockAtlas);
	atlas->Bind();

	// Set uniforms
	overlay_shader->SetUniform("u_ViewProjection", m_Camera->view_projection);
	overlay_shader->SetUniform("u_Brightness", m_World->GetBrightness());
	overlay_shader->SetUniform("u_Atlas", atlas);

	const TextureIDs& texture_ids = Block::GetBreakTextureIDs(m_Hand.break_frame);
	overlay_shader->SetUniform("u_TextureIDs", texture_ids.List());

	// Calculate model
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, m_SelectorPosition + glm::vec3(0.5f));
	model = glm::scale(model, glm::vec3(1.01f));
	overlay_shader->SetUniform("u_Model", model);

	MeshType type = Block::HasProperty(m_World->GetVoxel(m_Hand.break_position).id, BlockProperty_CrossMesh) ? MeshType::CrossMesh : MeshType::Block;
	Ref<VertexArray>& block_mesh = Game::GetMesh(type);
	block_mesh->Bind();
	glDrawElements(GL_TRIANGLES, block_mesh->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
	block_mesh->Unbind();

	atlas->Unbind();
	overlay_shader->Unbind();
}
void Player::RenderSelector() {
	if (!m_ShowSelector) return;
	Ref<Shader>& selector_shader = Game::GetShader(ShaderType::Selector);
	selector_shader->Bind();
	selector_shader->SetUniform("u_ViewProjection", m_Camera->view_projection);

	glm::mat4 model = glm::translate(glm::mat4(1.0f), m_SelectorPosition + glm::vec3(0.5f));
	selector_shader->SetUniform("u_Model", model);

	Ref<VertexArray>& wire_selector_mesh = Game::GetMesh(MeshType::WireSelector);
	wire_selector_mesh->Bind();
	glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, nullptr);
	wire_selector_mesh->Unbind();

	selector_shader->Unbind();
}

void Player::UpdateHand(float delta_time) {
	//if (m_Hotbar.GetSelected().id == 0 || m_Hotbar.GetSelected().id == Item::InvalidID) return;
	// Follow camera
	m_Hand.direction += (m_Camera->direction - m_Hand.direction) * 24.0f * delta_time;

	m_Hand.animator.Update(delta_time);
}
void Player::RenderHand() {
	if (m_Hotbar.GetSelected().id == 0 || m_Hotbar.GetSelected().id == Item::InvalidID) RenderEmptyHand();
	else if (m_Hotbar.GetSelected().IsBlock()) RenderBlockHand();
	else RenderItemHand();
}
void Player::RenderBlockHand() {
	Ref<Shader>& block_preview_shader = Game::GetShader(ShaderType::BlockPreview);
	block_preview_shader->Bind();

	Ref<Texture>& atlas = Game::GetTexture(TextureType::BlockAtlas);
	atlas->Bind();

	// Set uniforms
	block_preview_shader->SetUniform("u_ViewProjection", m_Camera->view_projection);
	block_preview_shader->SetUniform("u_Brightness", m_World->GetBrightness());
	block_preview_shader->SetUniform("u_Atlas", atlas);

	const TextureIDs& texture_ids = Block::GetTextureIDs(m_Hotbar.GetSelected().id);
	block_preview_shader->SetUniform("u_TextureIDs", texture_ids.List());

	// Calculate model
	glm::mat4 hand_model = glm::mat4(1.0f);
	hand_model = glm::translate(hand_model, m_Camera->position);
	hand_model *= glm::inverse(glm::lookAt(glm::vec3(0.0f), -m_Hand.direction, glm::vec3(0, 1, 0)));
	hand_model = m_Hand.transform.GetMatrix(hand_model);
	block_preview_shader->SetUniform("u_Model", hand_model);

	// Draw
	MeshType type = MeshType::Block;
	if (Block::HasProperty(m_Hotbar.GetSelected().id, BlockProperty_CrossMesh)) type = MeshType::CrossMesh;
	Ref<VertexArray>& block_mesh = Game::GetMesh(type);
	block_mesh->Bind();
	glDisable(GL_CULL_FACE);
	glDrawElements(GL_TRIANGLES, block_mesh->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
	glEnable(GL_CULL_FACE);
	block_mesh->Unbind();

	atlas->Unbind();
	block_preview_shader->Unbind();
}
void Player::RenderItemHand() {
	RenderEmptyHand();

	Ref<Shader>& held_item_shader = Game::GetShader(ShaderType::HeldItem);
	held_item_shader->Bind();

	Ref<Texture>& item_atlas = Game::GetTexture(TextureType::ItemAtlas);
	item_atlas->Bind();

	// Set uniforms
	held_item_shader->SetUniform("u_ViewProjection", m_Camera->view_projection);
	held_item_shader->SetUniform("u_Brightness", m_World->GetBrightness());
	held_item_shader->SetUniform("u_ItemAtlas", item_atlas);
	held_item_shader->SetUniform("u_TextureID", Item::GetTextureID(m_Hotbar.GetSelected().id));

	Transform hand_offset;
	hand_offset.position = glm::vec3(0.0f, 0.0f, -0.4f);
	hand_offset.rotation = glm::vec3(-1.7f, 0.1f, -0.75f);
	Transform item_offset;
	item_offset.position = glm::vec3(0.8f, -0.8f, 0.8f);
	item_offset.rotation = glm::vec3(glm::radians(-45.f), glm::radians(90.f), glm::radians(180.f));
	item_offset.scale    = glm::vec3(0.75f);

	// Calculate model
	glm::mat4 item_model = glm::mat4(1.0f);
	item_model = glm::translate(item_model, m_Camera->position);
	item_model *= glm::inverse(glm::lookAt(glm::vec3(0.0f), -m_Hand.direction, glm::vec3(0, 1, 0)));
	item_model = m_Hand.transform.GetMatrix(item_model);
	item_model = hand_offset.GetMatrix(item_model);
	item_model = item_offset.GetMatrix(item_model);
	held_item_shader->SetUniform("u_Model", item_model);

	// Draw
	Ref<VertexArray>& item_mesh = Game::GetMesh(MeshType::Quad);
	item_mesh->Bind();
	glDisable(GL_CULL_FACE);
	glDrawElements(GL_TRIANGLES, item_mesh->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
	glEnable(GL_CULL_FACE);
	item_mesh->Unbind();

	item_atlas->Unbind();
	held_item_shader->Unbind();
}
void Player::RenderEmptyHand() {
	Ref<Shader>& hand_shader = Game::GetShader(ShaderType::Hand);
	hand_shader->Bind();

	Ref<Texture>& skin = Game::GetTexture(TextureType::Skin);
	skin->Bind();

	// Set uniforms
	hand_shader->SetUniform("u_ViewProjection", m_Camera->view_projection);
	hand_shader->SetUniform("u_Brightness", m_World->GetBrightness());
	hand_shader->SetUniform("u_Texture", skin);

	Transform hand_offset;
	hand_offset.position = glm::vec3(0.0f, 0.0f, -0.4f);
	hand_offset.rotation = glm::vec3(-1.7f, 0.1f, -0.75f);

	// Calculate model
	glm::mat4 hand_model = glm::mat4(1.0f);
	hand_model = glm::translate(hand_model, m_Camera->position);
	hand_model *= glm::inverse(glm::lookAt(glm::vec3(0.0f), -m_Hand.direction, glm::vec3(0, 1, 0)));
	hand_model = m_Hand.transform.GetMatrix(hand_model);
	hand_model = hand_offset.GetMatrix(hand_model);
	hand_shader->SetUniform("u_Model", hand_model);

	// Draw
	Ref<VertexArray>& hand_mesh = Game::GetMesh(MeshType::Hand);
	hand_mesh->Bind();
	glDisable(GL_CULL_FACE);
	glDrawElements(GL_TRIANGLES, hand_mesh->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
	glEnable(GL_CULL_FACE);
	hand_mesh->Unbind();

	skin->Unbind();
	hand_shader->Unbind();
}

void Player::DropItem() {
	Item& selected = m_Hotbar.GetSelected();
	if (selected.count == 0) return;

	m_World->CreateItem({ selected.id, 1 }, m_Position + m_CameraOffset, m_Camera->direction * m_ItemDropForce, true);
	m_Hand.animator.Play("Swing");

	selected.count -= 1;
	if (selected.count == 0) selected = Item::Invalid;
}

// Animations
void Hand::Swing() {
	animator.Play("Swing");
}
void Hand::Hit() {
	animator.Play("Hit", 0.59f, true);
}
void Hand::Place() {
	animator.Play("Hit", 0.59f);
}

Hand::Hand() : animator(transform) {
	transform.position = glm::vec3(-0.2f, -0.2f, 0.3f);
	transform.rotation = glm::vec3(-0.1f, 0.7f, 0.0f);
	transform.scale    = glm::vec3(0.18f);

	auto cubic_ease_out = [](const Transform& t1, const Transform& t2, float time) {
		Transform result;
		float t = time * (3.0f - time * time) * 0.5f;
		result.position = (t2.position - t1.position) * t + t1.position;
		result.rotation = (t2.rotation - t1.rotation) * t + t1.rotation;
		result.scale = (t2.scale - t1.scale) * t + t1.scale;
		return result;
	};

	Animation sway_animation;
	sway_animation.function = Hand::SwayAnimationFunc;
	sway_animation.end_frame = { transform, 1.0f, cubic_ease_out };
	animator.AddAnimation("Sway", sway_animation);

	Animation swing_animation;
	swing_animation.key_frames = {
		{ // Frame #1	
			{
				glm::vec3(-0.3f, -0.185f, 0.3f),
				glm::vec3(-0.255f, 0.7f, 0.0f),
				transform.scale
			}, 0.04f
		},
		{ // Frame #2	
			{
				glm::vec3(-0.175f, -0.275f, 0.465f),
				glm::vec3(0.21f, 0.7f, -0.21f),
				transform.scale
			}, 0.03f
		},
		{ // Frame #3	
			{
				glm::vec3(-0.11f, -0.355f, 0.41f),
				glm::vec3(0.93f, 0.57f, -0.14f),
				transform.scale
			}, 0.02f
		},
		{ // Frame #3
			transform,
			0.05f
		}
	};
	swing_animation.end_frame = { transform, 0.1f, cubic_ease_out };
	animator.AddAnimation("Swing", swing_animation);
	
	Animation hit_animation;
	hit_animation.key_frames = {
		{ // Frame #1	
			{
				glm::vec3(-0.21f, -0.18f, 0.3f),
				glm::vec3(-0.255f, 0.7f, 0.0f),
				transform.scale
			}, 0.04f
		},
		{ // Frame #2	
			{
				glm::vec3(-0.175f, -0.265f, 0.465f),
				glm::vec3(0.21f, 0.7f, -0.21f),
				transform.scale
			}, 0.03f
		},
		{ // Frame #3	
			{
				glm::vec3(-0.11f, -0.315f, 0.41f),
				glm::vec3(0.93f, 0.57f, -0.14f),
				transform.scale
			}, 0.02f
		},
		{ // Frame #4
			transform,
			0.05f
		}
	};
	hit_animation.end_frame = { transform, 0.05f, cubic_ease_out };
	animator.AddAnimation("Hit", hit_animation);
	
	Animation swap_items_animation;
	swap_items_animation.key_frames = {
		{ // Frame #1	
			{
				glm::vec3(-0.2f, -0.3f, 0.3f),
				transform.rotation,
				transform.scale
			}, 0.04f, Transform::CubicInterp
		},
		{ // Frame #4
			transform,
			0.05f
		}
	};
	swap_items_animation.end_frame = { transform, 0.05f, cubic_ease_out };
	animator.AddAnimation("SwapItems", swap_items_animation);
}
Transform Hand::SwayAnimationFunc(float speed, float time) {
	Transform result;
	result.scale = glm::vec3(0.0f);
	result.position.x = cos(time * speed) * 0.025f;
	result.position.y = -abs(sin(time * speed) * 0.025f);
	return result;
}

