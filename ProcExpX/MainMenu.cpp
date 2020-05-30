#include "pch.h"
#include "MainMenu.h"

void BuildMainMenu() {
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Exit", nullptr, nullptr)) {
				::PostQuitMessage(0);
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("View")) {
			ImGui::MenuItem("Tool Bar");
			ImGui::MenuItem("Status Bar");

			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}