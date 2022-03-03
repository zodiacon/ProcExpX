#pragma once

#include "WinSys.h"
#include "ProcessInfoEx.h"
#include "MessageBox.h"
#include "ProcessProperties.h"
#include <d3d11_1.h>

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
	void BuildProcessMenu();
	void BuildToolBar();

	void BuildPriorityClassMenu(WinSys::ProcessInfo* pi);
	bool GotoFileLocation(WinSys::ProcessInfo* pi);
	void TogglePause();
	void BuildPropertiesWindow(ProcessProperties* props);
	
	std::shared_ptr<ProcessProperties> GetProcessProperties(WinSys::ProcessInfo* pi);
	std::shared_ptr<ProcessProperties> GetOrAddProcessProperties(const std::shared_ptr<WinSys::ProcessInfo>& pi);

	static CStringA ProcessAttributesToString(ProcessAttributes attributes);
	
private:
	CComPtr<ID3D11ShaderResourceView> m_spImage;
	WinSys::ProcessManager& _pm;
	DWORD64 _tick = 0;
	char _filterText[16]{};
	std::vector<std::shared_ptr<WinSys::ProcessInfo>> _processes;
	mutable std::unordered_map<WinSys::ProcessOrThreadKey, ProcessInfoEx> _processesEx;
	std::unordered_map<WinSys::ProcessOrThreadKey, std::shared_ptr<ProcessProperties>> _processProperties;
	const ImGuiTableColumnSortSpecs* _specs = nullptr;
	std::shared_ptr <WinSys::ProcessInfo> _selectedProcess;
	int _updateInterval = 1000, _oldInterval;
	TabManager& _tm;
	bool _modalOpen : 1 = false, _killFailed : 1 = false;
};
