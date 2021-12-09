#pragma once

#include "imgui.h"

namespace ImGui {
	bool ButtonEnabled(const char* label, bool enabled, const ImVec2& size = ImVec2(0, 0));
	bool ButtonColoredEnabled(const char* label, const ImVec4& color, bool enabled, const ImVec2& size = ImVec2(0, 0));
};

