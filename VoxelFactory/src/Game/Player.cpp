#include "Game/Player.h"
#include "Game/Game.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

Player::Player(const Ref<Camera>& camera, const Ref<World>& world)
	: m_Camera(camera), m_World(world) {

	m_Position = m_Camera->position;
	m_SelectorPosition = glm::vec3(0.0f);
	m_ShowSelector = false;

	m_MouseSensitivity = 0.1f;
	m_Speed = 24.0f;
	m_MouseLClickLast = false;
	m_MouseRClickLast = false;

	InitSelector();
}

void Player::Update(float delta_time, glm::vec2& last_mouse_pos, const Ref<Window>& window) {

	GLFWwindow* window_handle = (GLFWwindow*)window->GetHandle();
	
	bool mouse_left_click = glfwGetMouseButton(window_handle, GLFW_MOUSE_BUTTON_LEFT);
	if (mouse_left_click && !m_MouseLClickLast) {
		HandleLeftClick();
		m_MouseLClickLast = true;
	}
	else if (!mouse_left_click && m_MouseLClickLast) {
		m_MouseLClickLast = false;
	}
	bool mouse_right_click = glfwGetMouseButton(window_handle, GLFW_MOUSE_BUTTON_RIGHT);
	if (mouse_right_click && !m_MouseRClickLast) {
		HandleRightClick();
		m_MouseRClickLast = true;
	}
	else if (!mouse_right_click && m_MouseRClickLast) {
		m_MouseRClickLast = false;
	}

	// Camera position input
	glm::vec3 camera_up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 camera_right = glm::normalize(glm::cross(m_Camera->direction, camera_up));
	glm::vec3 delta_position = glm::vec3(0.0f);
	if (glfwGetKey(window_handle, GLFW_KEY_W))          delta_position += m_Camera->direction * m_Speed * delta_time;
	if (glfwGetKey(window_handle, GLFW_KEY_S))          delta_position -= m_Camera->direction * m_Speed * delta_time;
	if (glfwGetKey(window_handle, GLFW_KEY_A))          delta_position -= camera_right * m_Speed * delta_time;
	if (glfwGetKey(window_handle, GLFW_KEY_D))          delta_position += camera_right * m_Speed * delta_time;
	if (glfwGetKey(window_handle, GLFW_KEY_SPACE))      delta_position += camera_up * m_Speed * delta_time;
	if (glfwGetKey(window_handle, GLFW_KEY_LEFT_SHIFT)) delta_position -= camera_up * m_Speed * delta_time;

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
	constexpr float c_MaxPitch =  PIHalf - (PI / 180.0f); // 89 degrees
	constexpr float c_MinPitch = -PIHalf + (PI / 180.0f); // -89 degrees
	m_Camera->eular.x = std::min(c_MaxPitch, std::max(m_Camera->eular.x, c_MinPitch));

	if (delta_position != glm::vec3(0.0f) || delta_mouse != glm::vec2(0.0f)) {
		m_Camera->direction = Camera::EulerDirection(m_Camera->eular.x, m_Camera->eular.y);
		m_Camera->position += delta_position;
		m_Camera->UpdateView();
	}

	RayResultData ray_result = m_World->CastRay({ m_Camera->position, m_Camera->direction });
	m_ShowSelector = ray_result;
	m_SelectorPosition = ray_result.voxel_position;

	RenderSelector();
}

void Player::HandleLeftClick() {
	RayResultData ray_data = m_World->CastRay({ m_Camera->position, m_Camera->direction });
	if (!ray_data) return;
	
	// Delete voxel
	glm::vec3 local_position = Chunk::GetBlockLocalPosition(ray_data.voxel_position);
	glm::vec3 chunk_position = Chunk::GetBlockChunkPosition(ray_data.voxel_position);
	m_World->GetChunk(chunk_position)->At(local_position).id = 0;
	
	// Rebuild affected chunks
	m_World->RebuildChunk(chunk_position);
	if (local_position.x == 0) m_World->RebuildChunk({ chunk_position.x - 1, 0, chunk_position.z });
	else if (local_position.x == Chunk::ChunkLength - 1) m_World->RebuildChunk({ chunk_position.x + 1, 0, chunk_position.z });
	if (local_position.z == 0) m_World->RebuildChunk({ chunk_position.x, 0, chunk_position.z - 1 });
	else if (local_position.z == Chunk::ChunkLength - 1) m_World->RebuildChunk({ chunk_position.x, 0, chunk_position.z + 1 });
}
void Player::HandleRightClick() {
	RayResultData ray_data = m_World->CastRay({ m_Camera->position, m_Camera->direction });
	if (!ray_data) return;

	// Place dirt block
	glm::vec3 voxel_position = ray_data.voxel_position + ray_data.normal;
	glm::vec3 local_position = Chunk::GetBlockLocalPosition(voxel_position);
	glm::vec3 chunk_position = Chunk::GetBlockChunkPosition(voxel_position);
	m_World->GetChunk(chunk_position)->At(local_position).id = 3;

	// Rebuild affected chunks
	m_World->RebuildChunk(chunk_position);
}
void Player::InitSelector() {
	m_SelectorMesh = CreateRef<VertexArray>();
	m_SelectorMesh->Bind();

	Ref<VertexBuffer> vertex_buffer = CreateRef<VertexBuffer>(c_SelectorVertices, 48 * sizeof(float));
	vertex_buffer->Bind();
	constexpr size_t c_Stride = 6 * sizeof(float);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, c_Stride, (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, c_Stride, (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
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