#pragma once

#include <Windows.h>
#include "ProcessManager.h"
#include "TabManager.h"
#include <memory>

struct ImFont;

class Globals {
public:
	Globals(HWND hWnd);

	static Globals& Get();
	ImFont* MonoFont{ nullptr };
	ImFont* RegFont{ nullptr };
	HWND GetMainHwnd() const;

	WinSys::ProcessManager ProcMgr;
	TabManager& GetTabManager();

private:
	inline static Globals* _globals{ nullptr };
	std::unique_ptr<TabManager> _tabs;
	HWND _hwnd;
};

