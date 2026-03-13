#pragma once
#include "Core/Core.h"
#include "Core/Window.h"

class ImGuiHandler {
public:
	static void Init(const Ref<Window>& window);

	static void StartFrame();
	static void EndFrame();
};