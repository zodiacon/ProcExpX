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
	void BuildTabs();
	void BuildOptionsMenu();

private:
	ProcessesView _procView;
	Theme _theme = Theme::Dark;
};

