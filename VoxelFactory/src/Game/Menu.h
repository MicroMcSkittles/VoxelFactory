#pragma once
#include "Game/UI.h"

class Menu {
public:
	Menu(const std::string& title);
	~Menu() { }

	void AddButton(const Button& button);
	void SameLine();
	void End();

	void Update();
	void Render();

	std::string& GetTitle() { return m_Title; }

private:
	std::string m_Title;
	std::vector<std::vector<Button>> m_Buttons;

	glm::vec2 m_ButtonCursor;
	float m_MenuWidth;
	float m_MenuPadding;
	glm::vec2 m_ButtonPadding;
	float m_ButtonHeight;

};