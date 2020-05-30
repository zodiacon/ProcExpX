#include "pch.h"
#include "ImGuiExt.h"

namespace ImGui {
	bool ButtonEnabled(const char* label, bool enabled, const ImVec2& size) {
		if (!enabled) {
			PushStyleColor(ImGuiCol_Text, GetStyle().Colors[ImGuiCol_TextDisabled]);
			PushStyleColor(ImGuiCol_ButtonHovered, GetStyle().Colors[ImGuiCol_Button]);
		}
		auto clicked = Button(label, size);
		if (!enabled)
			PopStyleColor(2);

		return enabled && clicked;
	}

	bool ButtonColoredEnabled(const char* label, const ImVec4& color, bool enabled, const ImVec2& size) {
		return false;
	}
}