#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <wil\resource.h>
#include "WinSys.h"
#include "ProcessInfoEx.h"
#include "MessageBox.h"

struct ImGuiTableSortSpecsColumn;

enum class Theme {
	Classic,
	Light,
	Dark,
	Custom
};

class ProcessesView {
public:
	ProcessesView(HWND hMainWnd);
	void BuildWindow();

private:
	void DoSort(int col, bool asc);
	ProcessInfoEx& GetProcessInfoEx(WinSys::ProcessInfo* pi) const;
	void DoUpdate();
	bool KillProcess(uint32_t id);
	bool TryKillProcess(WinSys::ProcessInfo* pi);

	void BuildTable();
	void BuildOptionsMenu();
	void BuildViewMenu();
	void BuildFileMenu();
	void BuildProcessMenu();

private:
	WinSys::ProcessManager _pm;
	DWORD64 _tick = 0;
	std::vector<std::shared_ptr<WinSys::ProcessInfo>> _processes;
	mutable std::unordered_map<WinSys::ProcessOrThreadKey, ProcessInfoEx> _processesEx;
	const ImGuiTableSortSpecsColumn* _specs = nullptr;
	HWND _hMainWnd;
	std::shared_ptr <WinSys::ProcessInfo> _selectedProcess;
	int _updateInterval = 1000;
	Theme _theme = Theme::Dark;
	bool _modalOpen = false;
};
