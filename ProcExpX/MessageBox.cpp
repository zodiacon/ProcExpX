#include "pch.h"
#include "MessageBox.h"

using namespace ImGui;

MessageBoxResult SimpleMessageBox::ShowModal(const char* title, const char* text, MessageBoxButtons buttons) {
	bool ret = false;
	if(!IsPopupOpen(title))
		OpenPopup(title);
	auto result = MessageBoxResult::StillOpen;
	auto count = 1;

	switch (buttons) {
		case MessageBoxButtons::YesNo:
		case MessageBoxButtons::OkCancel:
			count = 2;
			break;
	}

	if (BeginPopupModal(title, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		auto winWidth = GetWindowSize().x;
		Text(text);
		Dummy(ImVec2(0, 6));
		Separator();
		Dummy(ImVec2(0, 10));
		NewLine();

		auto width = 100.0f;
		SameLine((winWidth - width * count - GetStyle().ItemSpacing.x * (count - 1)) / 2);
		if (Button("OK", ImVec2(width, 0))) {
			CloseCurrentPopup();
			result = MessageBoxResult::OK;
		}
		if (count > 1) {
			SetItemDefaultFocus();
			SameLine();
			if (Button("Cancel", ImVec2(width, 0))) {
				CloseCurrentPopup();
				result = MessageBoxResult::Cancel;
			}
		}
		if (IsKeyPressed(VK_ESCAPE)) {
			CloseCurrentPopup();
			result = MessageBoxResult::Cancel;
		}
		EndPopup();
	}
	return result;
}
