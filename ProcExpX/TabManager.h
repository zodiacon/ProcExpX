#pragma once

#include "ProcessesView.h"

enum class Theme {
	Classic,
	Light,
	Dark,
	Custom
};

class TabManager {
public:
	TabManager();

	void BuildMainMenu();
	void BuildTabs();
	void BuildOptionsMenu();
	void BuildFileMenu();
	void BuildWindowMenu();
	void BuildHelpMenu();

	void AddWindow(std::shared_ptr<WindowProperties> window);

private:
	std::unordered_map<std::string, std::shared_ptr<WindowProperties>> _windows;
	ProcessesView _procView;
	Theme _theme = Theme::Dark;
};

