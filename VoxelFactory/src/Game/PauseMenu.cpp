#include "Game/PauseMenu.h"
#include "Game/Game.h"
#include "Game/UI.h"

PauseMenu::PauseMenu() {
	m_Padding = 0.15f;
	m_ButtonPadding = 0.25f;
	m_ReturnButton  = CreateRef<Button>("Return",  glm::vec3(0.0f), glm::vec2(8.5f, 0.4f));
	m_OptionsButton = CreateRef<Button>("Options", glm::vec3(0.0f), glm::vec2(8.5f, 0.4f));
	m_QuitButton    = CreateRef<Button>("Quit",    glm::vec3(0.0f), glm::vec2(8.5f, 0.4f));
}

void PauseMenu::Update(float delta_time) {
	glm::vec2 screen_min = UI::GetScreenMin();
	glm::vec2 screen_max = UI::GetScreenMax();

	// Set button positions
	glm::vec3 button_cursor = {
		0.0f,
		screen_max.y - (screen_max.y - screen_min.y) * m_Padding - 2.5f,
		0.0f
	};
	m_ReturnButton->position = button_cursor;
	button_cursor.y -= (m_ReturnButton->size.y + m_ReturnButton->size.y * m_ButtonPadding) * 2.0f;
	m_OptionsButton->position = button_cursor;
	button_cursor.y -= (m_OptionsButton->size.y + m_OptionsButton->size.y * m_ButtonPadding) * 2.0f;
	m_QuitButton->position = button_cursor;

	// Update button states
	m_ReturnButton->Update();
	m_OptionsButton->Update();
	m_QuitButton->Update();

	// Process button states
	if (m_ReturnButton->pressed) {
		Game::Get()->Resume();
	}

	if (m_QuitButton->pressed) {
		Game::Get()->Quit();
	}
}
void PauseMenu::RenderUI() {
	glm::vec2 screen_min = UI::GetScreenMin();
	glm::vec2 screen_max = UI::GetScreenMax();

	// Tint screen
	glm::vec2 screen_tint_size = (screen_max - screen_min) * 0.5f;
	UI::ColoredQuad(glm::vec3(0.0f, 0.0f, -1.0f), screen_tint_size, glm::vec4(0.0f,0.0f,0.0f,0.65f));

	// Paused text
	glm::vec3 paused_text_position = {
		0.0f,
		screen_max.y - (screen_max.y - screen_min.y) * m_Padding,
		0.0f
	};
	UI::Text("Game Paused", paused_text_position, glm::vec2(0.5f), TextAlignment_Middle, glm::vec3(1.0f), glm::vec4(0.0f));
	
	// Show buttons
	m_ReturnButton->Render();
	m_OptionsButton->Render();
	m_QuitButton->Render();
}