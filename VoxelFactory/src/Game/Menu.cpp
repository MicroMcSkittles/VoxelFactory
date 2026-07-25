#include "Game/Menu.h"

Menu::Menu(const std::string& title): m_Title(title) {
	m_ButtonCursor = glm::vec2(0.0f, 0.0f);
	m_MenuWidth = 8.5f;
	m_MenuPadding = 0.15f;
	m_ButtonPadding = glm::vec2(0.25f);
	m_ButtonHeight = 0.4f;
}

void Menu::AddButton(const Button& button) {
	if (m_Buttons.size() == (size_t)m_ButtonCursor.y) m_Buttons.push_back({});
	m_Buttons[m_ButtonCursor.y].push_back(button);
	m_ButtonCursor.y += 1;
}
void Menu::SameLine() {
	m_ButtonCursor.y -= 1;
}
void Menu::End() {
	glm::vec2 screen_min = UI::GetScreenMin();
	glm::vec2 screen_max = UI::GetScreenMax();

	float y_pos = screen_max.y - (screen_max.y - screen_min.y) * m_MenuPadding - 2.5f;
	for (std::vector<Button>& button_line : m_Buttons) {
		float button_width = (m_MenuWidth - m_ButtonPadding.x * (button_line.size() - 1) * 0.5f) / button_line.size();
		float x_pos = 0.0f;
		for (Button& button : button_line) {
			button.size.x = button_width;
			button.size.y = m_ButtonHeight;
			button.position.x = x_pos - m_MenuWidth;
			button.position.y = y_pos;
			x_pos += button_width * 2.0f + m_ButtonPadding.x;
		}
		y_pos -= m_ButtonHeight * 2.0f + m_ButtonPadding.y;
	}
}

void Menu::Update() {
	for (std::vector<Button>& button_line : m_Buttons) {
		for (Button& button : button_line) {
			button.Update();
		}
	}
}
void Menu::Render() {

	glm::vec2 screen_min = UI::GetScreenMin();
	glm::vec2 screen_max = UI::GetScreenMax();

	// Tint screen
	glm::vec2 screen_tint_size = (screen_max - screen_min) * 0.5f;
	UI::ColoredQuad(glm::vec3(0.0f, 0.0f, -1.0f), screen_tint_size, glm::vec4(0.0f, 0.0f, 0.0f, 0.65f));

	// Title text
	glm::vec3 title_text_position = {
		0.0f,
		screen_max.y - (screen_max.y - screen_min.y) * m_MenuPadding,
		0.0f
	};
	UI::Text(m_Title, title_text_position, glm::vec2(0.5f), TextAlignment_Middle, glm::vec3(1.0f), glm::vec4(0.0f));

	// Render buttons
	for (std::vector<Button>& button_line : m_Buttons) {
		for (Button& button : button_line) {
			button.Render();
		}
	}
}