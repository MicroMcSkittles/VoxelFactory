#pragma once
#include "Core/Core.h"
#include "Core/Window.h"
#include "Game/World.h"
#include "Game/Inventory.h"
#include "Renderer/Camera.h"
#include "Renderer/Buffers.h"
#include "Renderer/Shader.h"

struct Hand {
	const glm::vec3 default_offset   = glm::vec3(-0.2f, -0.2f, 0.3f);
	const glm::vec3 default_size     = glm::vec3(0.18f);
	const glm::vec3 default_rotation = glm::vec3(-0.1f, 0.7f, 0.0f);

	glm::vec3 offset    = glm::vec3(0.0f);
	glm::vec3 rotation  = glm::vec3(0.0f);
	glm::vec3 direction = glm::vec3(0.0f);

	// Animations
	void Swing();
	void Hit();
};

struct Hotbar {
	int selected;
	Ref<Inventory> inventory;

	Hotbar();
	Item& GetSelected();
	Item& Get(int index);

	const inline static int Width = 9;
};

class Player {
public:

	Player(const glm::vec3& position, const Ref<Camera>& camera, const Ref<World>& world);

	void ShowImGui();
	void Update(float delta_time, glm::vec2& last_mouse_pos, const Ref<Window>& window);
	void Render();
	void RenderUI();

	std::string StatsText();

	bool OnKey(int key, int action, int mods);
	void OnLeftClick();
	void OnRightClick();
	void OnScroll(float delta);

	void OpenInventory(const Ref<Inventory>& other);
	void OpenInventory();
	void CloseInventory();

	// Attempts to push items to the hotbar then main inventory
	void PushInventoryItems(Item& item);
	// Returns true if there is space for the item in either hotbar or inventory
	bool HasItemSpace(const Item& item);

	glm::vec3 GetPosition() { return m_Position; }
	glm::vec3 GetCameraPosition() { return m_Position + m_CameraOffset; }

private:

	// Update velocity based on input
	void Input(float delta_time, const Ref<Window>& window);
	// Update camera rotation based on mouse movement
	void CameraInput(glm::vec2& last_mouse_pos, const Ref<Window>& window);

	void InitSelector();
	void RenderSelector();

	void UpdateHand(float delta_time);
	void RenderHand();

	void RenderInventoryUI();
	void DropItem();

private:
	
	// General
	Ref<World> m_World;
	Ref<Camera> m_Camera;

	Hand m_Hand;
	Hotbar m_Hotbar;
	Ref<Inventory> m_Inventory;
	Ref<InventoryHandler> m_InventoryHandler;

	glm::vec3 m_Position;
	glm::vec3 m_CameraOffset;
	
	// Physics
	glm::vec3 m_ColliderOffset;
	AABB m_Collider;
	glm::vec3 m_Velocity;

	float m_GroundCheckDist;
	bool m_OnGround;

	// *Move to Game class
	float m_GravitationalConstant;
	float m_Drag;

	// Selector * Make seporate struct
	Ref<VertexArray> m_SelectorMesh;
	glm::vec3 m_SelectorPosition;
	bool m_ShowSelector;

	// Controls
	bool m_Flight;
	bool m_OpenInventory;
	bool m_EnableCollision;
	float m_MouseSensitivity;
	float m_WalkSpeed;
	float m_SprintMultiplier;
	float m_Speed;
	float m_JumpForce;
	float m_Reach;
	float m_MovementTime;
	float m_ItemDropForce;

private:
	const inline static float c_SelectorVertices[] = {
		-0.52f, -0.52f, -0.52f, 0.0f, 0.0f, 0.0f,
		-0.52f,  0.52f, -0.52f, 0.0f, 0.0f, 0.0f,

		 0.52f, -0.52f, -0.52f, 0.0f, 0.0f, 0.0f,
		 0.52f,  0.52f, -0.52f, 0.0f, 0.0f, 0.0f,

		-0.52f, -0.52f,  0.52f, 0.0f, 0.0f, 0.0f,
		-0.52f,  0.52f,  0.52f, 0.0f, 0.0f, 0.0f,

		 0.52f, -0.52f,  0.52f, 0.0f, 0.0f, 0.0f,
		 0.52f,  0.52f,  0.52f, 0.0f, 0.0f, 0.0f
	};
	const inline static uint32_t c_SelectorIndices[] = {
		0, 1, 1, 3, 3, 2, 2, 0,
		0, 4, 1, 5, 2, 6, 3, 7,
		4, 5, 5, 7, 7, 6, 6, 4
	};
};