#include "Game/Player.h"
#include "Game/Game.h"
#include "Game/UI.h"
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

void Hand::InitMesh() {
	const float c_Vertices[] = {
		// Front
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f,
	     0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,
	     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f,
	     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f,
	    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,
	    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f,
	    							 
	    // Back						 
	     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,
	     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f,
	    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,
	    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,
	    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f,
	     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,
	    							 
	    // Left						 
	     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
	     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
	     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
	     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
	     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
	     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
	    							 
	    // Right					 
	    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
	    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
	    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
	    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
	    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
	    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
	    
	    // Top
	     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f,
	     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f,
	    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f,
	    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f,
	    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
	     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f,
	    
	    // Bottom
	    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f,
	     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f,
	     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f,
	     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f,
	    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
	    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f
	};
	const uint32_t c_Indices[] = {
		0,  1,  2,  3,  4,  5,  // Front
		6,  7,  8,  9,  10, 11, // Back
		12, 13, 14, 15, 16, 17, // Left
		18, 19, 20, 21, 22, 23, // Right
		24, 25, 26, 27, 28, 29, // Top
		30, 31, 32, 33, 34, 35  // Bottom
	};
	const VertexLayout c_Layout = { {
		{ GL_FLOAT, 3 }, // a_Position
		{ GL_FLOAT, 3 }, // a_Normal
		{ GL_FLOAT, 2 }, // a_TexCoord
	} };

	empty_hand_mesh = CreateRef<VertexArray>();
	empty_hand_mesh->Bind();

	Ref<VertexBuffer> vertex_buffer = CreateRef<VertexBuffer>(c_Vertices, sizeof(c_Vertices), c_Layout);
	empty_hand_mesh->GetVertexBuffer() = vertex_buffer;

	Ref<IndexBuffer> index_buffer = CreateRef<IndexBuffer>(c_Indices, sizeof(c_Indices));
	empty_hand_mesh->GetIndexBuffer() = index_buffer;

	empty_hand_mesh->Unbind();
}

Player::Player(const glm::vec3& position, const Ref<Camera>& camera, const Ref<World>& world)
	: m_Position(position), m_Camera(camera), m_World(world) {

	m_Inventory = CreateRef<Inventory>(9, 3, 6.0f, 16.0f, 2.0f, glm::vec2(8, 30), Game::GetTexture(TextureType::Inventory));
	for (int i = 1; i < std::min((int)BlockID_Count, 9 * 3); i++) {
		int x = (i - 1) % 9;
		int y = (i - 1) / 9;
		m_Inventory->GetItem({ x, y }).id = i;
		m_Inventory->GetItem({ x, y }).count = Item::StackSize;
	}
	m_InventoryHandler = nullptr;

	m_Hand.direction = m_Camera->direction;
	m_Hand.offset = m_Hand.default_offset;
	m_Hand.rotation = m_Hand.default_rotation;
	m_Hand.InitMesh();

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
	m_NoClip = false;
	m_OpenInventory = false;
	m_EnableCollision = true;
	m_MouseSensitivity = 0.1f;
	m_WalkSpeed = 5.612f;
	m_Speed = m_WalkSpeed;
	m_SprintMultiplier = 1.5f;
	m_JumpForce = 0.125f;
	m_Reach = 7.0f;
	m_MovementTime = 0.0f;
	m_ItemDropForce = 0.125f;

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
	ImGui::Checkbox("Enable Collision", &m_EnableCollision);
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
	ImGui::DragFloat3("Hand Offset", &m_Hand.offset.x, 0.01f);
	ImGui::DragFloat3("Hand Rotation", &m_Hand.rotation.x, 0.1f);
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

void Player::Update(float delta_time, glm::vec2& last_mouse_pos, const Ref<Window>& window) {

	Input(delta_time, window);

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

	CameraInput(last_mouse_pos, window);

	// Selector ray
	CollisionResultData ray_result = m_World->CastRay({ m_Camera->position, m_Camera->direction });
	m_ShowSelector = (ray_result.hit && ray_result.dist <= m_Reach);
	m_SelectorPosition = ray_result.voxel_position;

	// Update hand
	UpdateHand(delta_time);
}
void Player::Render() {
	RenderHand();
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

	// Delete voxel
	m_World->BreakVoxel(m_SelectorPosition);
	m_Hand.Hit();
}
void Player::OnRightClick() {
	if (Game::GetState() == GameState::Menu && m_OpenInventory) m_InventoryHandler->OnRightClick();
	if (Game::GetState() != GameState::InGame) return;
	if (!m_ShowSelector) return;
	if (!m_Hotbar.GetSelected().IsBlock()) return;

	CollisionResultData ray_data = m_World->CastRay({ m_Camera->position, m_Camera->direction });

	// Find block oriantation
	glm::vec3 direction = -(m_Camera->direction * ray_data.dist);
	uint8_t block_id = (uint8_t)m_Hotbar.GetSelected().id;
	if (Block::HasProperty(block_id, BlockProperty_HasOrientation)) 
		block_id |= Block::CalculateOrientation(direction, block_id);

	// Place block from players hand
	glm::vec3 voxel = ray_data.voxel_position + ray_data.normal;
	if (m_World->WillIntersect(m_Collider, voxel)) return;
	m_World->SetVoxel(voxel, block_id);
	
	Item& selected = m_Hotbar.GetSelected();
	selected.count -= 1;
	if (selected.count == 0) selected = Item::Invalid;

	m_Hand.Hit();
}
void Player::OnScroll(float delta) {
	Item last_item = m_Hotbar.GetSelected();

	if (delta < 0.0f) m_Hotbar.selected += 1;
	else m_Hotbar.selected -= 1;

	if (m_Hotbar.selected < 0) m_Hotbar.selected = Hotbar::Width - 1;
	else if (m_Hotbar.selected >= Hotbar::Width) m_Hotbar.selected = 0;
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

void Player::Input(float delta_time, const Ref<Window>& window) {
	if (delta_time > 1.0f) return;
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

	if ((m_Velocity.x != 0.0f || m_Velocity.z != 0.0f) && m_OnGround) {
		m_MovementTime += delta_time;
	}
	else {
		m_MovementTime = 0.0f;
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
void Player::CameraInput(glm::vec2& last_mouse_pos, const Ref<Window>& window) {
	if (m_OpenInventory || !Game::IsMouseCaptured()) {
		m_Camera->position = m_Position + m_CameraOffset;
		m_Camera->UpdateView();
		return;
	}
	
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

	if (m_ShowSelector) {
		ss << "Looking at " << VEC3_STR(m_SelectorPosition) << "\n";
	}

	return ss.str();
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
}
void Player::RenderSelector() {
	if (!m_ShowSelector) return;
	Ref<Shader>& selector_shader = Game::GetShader(ShaderType::Selector);
	selector_shader->Bind();
	selector_shader->SetUniform("u_ViewProjection", m_Camera->view_projection);

	glm::mat4 model = glm::translate(glm::mat4(1.0f), m_SelectorPosition + glm::vec3(0.5f));
	selector_shader->SetUniform("u_Model", model);

	m_SelectorMesh->Bind();
	glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, nullptr);
	m_SelectorMesh->Unbind();

	selector_shader->Unbind();
}

void Player::UpdateHand(float delta_time) {
	if (m_Hotbar.GetSelected().id == 0 || m_Hotbar.GetSelected().id == Item::InvalidID) return;
	// Follow camera
	m_Hand.direction += (m_Camera->direction - m_Hand.direction) * 24.0f * delta_time;

	glm::vec3 target_offset = m_Hand.default_offset;

	if (m_MovementTime != 0.0f) {
		target_offset.x = m_Hand.default_offset.x + cos(m_MovementTime * m_Speed) * 0.025f;
		target_offset.y = m_Hand.default_offset.y - abs(sin(m_MovementTime * m_Speed) * 0.025f);
	}

	m_Hand.offset += (target_offset - m_Hand.offset) * 10.0f * delta_time;
	m_Hand.rotation += (m_Hand.default_rotation - m_Hand.rotation) * 10.0f * delta_time;
}
void Player::RenderHand() {
	if (m_Hotbar.GetSelected().id == 0 || m_Hotbar.GetSelected().id == Item::InvalidID) return;

	//Ref<Shader>& block_preview_shader = Game::GetShader(ShaderType::Hand);
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
	hand_model = glm::translate(hand_model, m_Hand.offset);
	hand_model = glm::rotate(hand_model, m_Hand.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
	hand_model = glm::rotate(hand_model, m_Hand.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
	hand_model = glm::rotate(hand_model, m_Hand.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
	hand_model = glm::scale(hand_model, m_Hand.default_size);
	block_preview_shader->SetUniform("u_Model", hand_model);

	// Draw
	MeshType type = MeshType::Block;
	if (Block::HasProperty(m_Hotbar.GetSelected().id, BlockProperty_CrossMesh)) type = MeshType::CrossMesh;
	Ref<VertexArray>& block_mesh = Game::GetMesh(type);
	block_mesh->Bind();
	//m_Hand.empty_hand_mesh->Bind();
	glDisable(GL_CULL_FACE);
	//glDrawElements(GL_TRIANGLES, m_Hand.empty_hand_mesh->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
	glDrawElements(GL_TRIANGLES, block_mesh->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
	glEnable(GL_CULL_FACE);
	//m_Hand.empty_hand_mesh->Unbind();
	block_mesh->Unbind();

	atlas->Unbind();
	block_preview_shader->Unbind();
}

void Player::DropItem() {
	Item& selected = m_Hotbar.GetSelected();
	if (selected.count == 0) return;

	m_World->CreateItem({ selected.id, 1 }, m_Position + m_CameraOffset, m_Camera->direction * m_ItemDropForce, true);
	
	selected.count -= 1;
	if (selected.count == 0) selected = Item::Invalid;
}

// Animations
void Hand::Swing() {
	offset = glm::vec3(0.0f, -0.4f, 0.49f);
	rotation = glm::vec3(1.0f, 0.2f, 0.0f);
}
void Hand::Hit() {
	offset = glm::vec3(-0.2f, -0.5f, 0.49f);
	rotation = glm::vec3(1.0f, 0.2f, 0.0f);
}
