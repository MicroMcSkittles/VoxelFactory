#pragma once
#include "Game/UI.h"

class PauseMenu {
public:
	PauseMenu();
	~PauseMenu() { }

	void Update(float delta_time);
	void RenderUI();

private:
	float m_Padding;
	float m_ButtonPadding;
	Ref<Button> m_ReturnButton;
	Ref<Button> m_OptionsButton;
	Ref<Button> m_QuitButton;
};