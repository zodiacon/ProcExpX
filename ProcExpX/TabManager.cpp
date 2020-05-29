#include <Windows.h>
#include "TabManager.h"
#include "imgui.h"

using namespace ImGui;

TabManager::TabManager(HWND hwnd) : _hwnd(hwnd), _procView(*this) {
}

void TabManager::BuildTabs() {
	if (Begin("main", nullptr,
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar)) {
		auto size = GetIO().DisplaySize;
		SetWindowSize(size, ImGuiCond_Always);
		SetWindowPos(ImVec2(0, 0), ImGuiCond_Always);

		if (BeginTabBar("tabs", ImGuiTabBarFlags_Reorderable)) {
			if (BeginTabItem("Processes", nullptr, ImGuiTabItemFlags_None)) {
				_procView.BuildWindow();
				EndTabItem();
			}
			if (BeginTabItem("Services", nullptr, ImGuiTabItemFlags_None)) {
				EndTabItem();
			}
			if (BeginTabItem("Log", nullptr, ImGuiTabItemFlags_None)) {
				EndTabItem();
			}
			EndTabBar();
		}
	}
	End();
}

void TabManager::BuildOptionsMenu() {
	if (BeginMenu("Options")) {
		static bool alwaysOnTop;
		if (MenuItem("Always On Top", nullptr, &alwaysOnTop)) {
			::SetWindowPos(_hwnd, !alwaysOnTop ? HWND_NOTOPMOST : HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
		}
		Separator();
		if (BeginMenu("Theme")) {
			if (MenuItem("Classic", nullptr, _theme == Theme::Classic)) {
				StyleColorsClassic();
				_theme = Theme::Classic;
			}
			if (MenuItem("Light", nullptr, _theme == Theme::Light)) {
				StyleColorsLight();
				_theme = Theme::Light;
			}
			if (MenuItem("Dark", nullptr, _theme == Theme::Dark)) {
				StyleColorsDark();
				_theme = Theme::Dark;
			}

			ImGui::EndMenu();
		}

		ImGui::EndMenu();
	}
}
