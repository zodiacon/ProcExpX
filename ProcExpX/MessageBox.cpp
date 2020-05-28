#include "MessageBox.h"
#include "imgui.h"

using namespace ImGui;

MessageBoxResult SimpleMessageBox::ShowModal(const char* title, const char* text, MessageBoxButtons buttons) {
	bool ret = false;
	OpenPopup(title);
	auto result = MessageBoxResult::StillOpen;

	bool open = true;
	if (BeginPopupModal(title, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		Text(text);
		Dummy(ImVec2(0, 6));
		Separator();
		Dummy(ImVec2(0, 10));
		NewLine();

		SameLine((GetWindowSize().x - 200 - GetStyle().ItemSpacing.x) / 2);
		if (Button("OK", ImVec2(100, 0))) {
			CloseCurrentPopup();
			result = MessageBoxResult::OK;
		}
		SetItemDefaultFocus();
		SameLine();
		if (Button("Cancel", ImVec2(100, 0))) {
			CloseCurrentPopup();
			result = MessageBoxResult::Cancel;
		}

		EndPopup();
	}
	return result;
}
