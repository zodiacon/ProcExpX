#pragma once

#include "imgui.h"
#include <atlstr.h>

struct ProcessColor {
	ProcessColor(const char* name, const ImVec4& defaultColor, const ImVec4& defaultTextColor, bool enabled = true);

	CStringA Name;
	ImVec4 DefaultColor;
	ImVec4 Color;
	ImVec4 DefaultTextColor;
	ImVec4 TextColor;
	bool Enabled;
};

