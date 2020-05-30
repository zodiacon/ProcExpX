#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <wil\resource.h>
#include "WinSys.h"
#include "ProcessInfoEx.h"
#include "MessageBox.h"
#include <atlstr.h>

struct ImGuiTableSortSpecsColumn;
class TabManager;

class ProcessesView {
public:
	ProcessesView(TabManager& tm);
	void BuildWindow();

private:
	void DoSort(int col, bool asc);
	ProcessInfoEx& GetProcessInfoEx(WinSys::ProcessInfo* pi) const;
	void DoUpdate();
	bool KillProcess(uint32_t id);
	bool TryKillProcess(WinSys::ProcessInfo* pi, bool& success);

	void BuildTable();
	void BuildViewMenu();
	void BuildFileMenu();
	void BuildProcessMenu();
	void BuildToolBar();

	void BuildPriorityClassMenu(WinSys::ProcessInfo* pi);
	bool GotoFileLocation(WinSys::ProcessInfo* pi);
	void TogglePause();

	static CStringA ProcessAttributesToString(ProcessAttributes attributes);

private:
	WinSys::ProcessManager& _pm;
	DWORD64 _tick = 0;
	std::vector<std::shared_ptr<WinSys::ProcessInfo>> _processes;
	mutable std::unordered_map<WinSys::ProcessOrThreadKey, ProcessInfoEx> _processesEx;
	const ImGuiTableSortSpecsColumn* _specs = nullptr;
	std::shared_ptr <WinSys::ProcessInfo> _selectedProcess;
	int _updateInterval = 1000, _oldInterval;
	TabManager& _tm;
	bool _modalOpen : 1 = false, _killFailed : 1 = false;
};
