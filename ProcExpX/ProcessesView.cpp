#include "ProcessesView.h"
#include "imgui.h"
#include <Windows.h>
#include <TlHelp32.h>
#include <atlbase.h>
#include <algorithm>
#include "SortHelper.h"
#include <atltime.h>
#include <strsafe.h>
#include "Processes.h"

using namespace ImGui;

ProcessesView::ProcessesView(HWND hMainWnd) : _hMainWnd(hMainWnd) {
}

void ProcessesView::BuildWindow() {
	if (Begin("Processes", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar)) {
		auto size = GetIO().DisplaySize;
		SetWindowSize(size, ImGuiCond_Always);
		SetWindowPos(ImVec2(0, 0), ImGuiCond_Always);
		if (BeginMenuBar()) {
			BuildFileMenu();
			BuildViewMenu();
			BuildProcessMenu();
			BuildOptionsMenu();

			EndMenuBar();
		}

		BuildTable();
	}
	End();
}

void ProcessesView::DoSort(int col, bool asc) {
	std::sort(_processes.begin(), _processes.end(), [=](const auto& p1, const auto& p2) {
		switch (col) {
			case 0: return SortHelper::SortStrings(p1->GetImageName(), p2->GetImageName(), asc);
			case 1: return SortHelper::SortNumbers(p1->Id, p2->Id, asc);
			case 2: return SortHelper::SortNumbers(p1->SessionId, p2->SessionId, asc);
			case 3: return SortHelper::SortNumbers(p1->CPU, p2->CPU, asc);
			case 4: return SortHelper::SortNumbers(p1->ParentId, p2->ParentId, asc);
			case 5: return SortHelper::SortNumbers(p1->CreateTime, p2->CreateTime, asc);
			case 6: return SortHelper::SortNumbers(p1->PrivatePageCount, p2->PrivatePageCount, asc);
			case 7: return SortHelper::SortNumbers(p1->BasePriority, p2->BasePriority, asc);
		}
		return false;
		});
}

ProcessInfoEx& ProcessesView::GetProcessInfoEx(WinSys::ProcessInfo* pi) const {
	auto it = _processesEx.find(pi->Key);
	if (it != _processesEx.end())
		return it->second;

	ProcessInfoEx px(pi);
	_processesEx.insert({ pi->Key, std::move(px) });
	return GetProcessInfoEx(pi);
}

void ProcessesView::DoUpdate() {
	for (auto& pi : _pm.GetNewProcesses()) {
		_processes.push_back(pi);
		auto& px = GetProcessInfoEx(pi.get());
		px.New(3000);
	}

	for (auto& pi : _pm.GetTerminatedProcesses()) {
		auto& px = GetProcessInfoEx(pi.get());
		px.Term(3000);
	}
}

bool ProcessesView::KillProcess(uint32_t id) {
	auto process = WinSys::Process::OpenById(id, WinSys::ProcessAccessMask::Terminate);
	if (process == nullptr)
		return false;

	return process->Terminate();
}

bool ProcessesView::TryKillProcess(WinSys::ProcessInfo* pi) {
	_modalOpen = true;
	CStringA text;
	text.Format("Kill process %u (%ws)?",
		_selectedProcess->Id, _selectedProcess->GetImageName().c_str());

	auto result = SimpleMessageBox::ShowModal("Kill Process?", text, MessageBoxButtons::OkCancel);
	if (result != MessageBoxResult::StillOpen) {
		_modalOpen = false;
		if (result == MessageBoxResult::OK)
			KillProcess(_selectedProcess->Id);
		_selectedProcess.reset();
		return true;
	}
	return false;
}

void ProcessesView::BuildTable() {
	if (BeginTable("processes", 9, ImGuiTableFlags_BordersV | ImGuiTableFlags_Sortable | ImGuiTableFlags_ScrollFreeze2Columns |
		ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollFreezeTopRow | ImGuiTableFlags_Reorderable | ImGuiTableFlags_BordersVFullHeight |
		ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Scroll | ImGuiTableFlags_RowBg)) {

		//tableWidth = GetItemRectSize().x;
		//ATLTRACE(L"Size: %f,%f\n", tableWidth);

		TableSetupColumn("Name", ImGuiTableColumnFlags_None);
		TableSetupColumn("Id");
		TableSetupColumn("Session");
		TableSetupColumn("CPU (%)");
		TableSetupColumn("Parent Process");
		TableSetupColumn("Created", ImGuiTableColumnFlags_WidthFixed);
		TableSetupColumn("Private Bytes");
		TableSetupColumn("Priority");

		TableAutoHeaders();

		if (_updateInterval > 0 && ::GetTickCount64() - _tick >= _updateInterval) {
			auto empty = _processes.empty();
			if (empty) {
				_processes.reserve(1024);
				_processesEx.reserve(1024);
			}
			_pm.EnumProcesses();
			if (empty) {
				_processes = _pm.GetProcesses();
			}
			else {
				DoUpdate();
			}
			if (_specs)
				DoSort(_specs->ColumnIndex, _specs->SortDirection == ImGuiSortDirection_Ascending);
			_tick = ::GetTickCount64();
		}

		auto count = static_cast<int>(_processes.size());
		for (int i = 0; i < count; i++) {
			const auto& p = _processes[i];
			auto& px = GetProcessInfoEx(p.get());
			if (px.Update()) {
				// process terminated
				_processesEx.erase(p->Key);
				_processes.erase(_processes.begin() + i);
				i--;
				count--;
				continue;
			}
		}

		auto specs = TableGetSortSpecs();
		if (specs && specs->SpecsChanged) {
			_specs = specs->Specs;
			DoSort(_specs->ColumnIndex, _specs->SortDirection == ImGuiSortDirection_Ascending);
		}
		USES_CONVERSION;
		ImGuiListClipper clipper;
		const ImVec4 red(1, 0, 0, 1);
		const ImVec4 green(0, .5f, 0, 1);

		count = static_cast<int>(_processes.size());
		clipper.Begin(count);
		auto special = false;
		static char buffer[256];
		CStringA str;

		int popCount;
		static bool selected = false;

		while (clipper.Step()) {
			for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
				auto& p = _processes[i];
				auto& px = GetProcessInfoEx(p.get());

				TableNextRow();

				if (special)
					PopStyleColor(popCount);

				special = px.IsNew() || px.IsTerminated() || p == _selectedProcess;
				if (special) {
					if (p == _selectedProcess) {
						auto color = ImVec4(0, 0, 1, .8f);// GetStyle().Colors[ImGuiCol_TextSelectedBg];
						PushStyleColor(ImGuiCol_TableRowBg, color);
						PushStyleColor(ImGuiCol_TableRowBgAlt, color);
						PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
						popCount = 3;
					}
					else {
						PushStyleColor(ImGuiCol_TableRowBg, px.IsTerminated() ? red : green);
						PushStyleColor(ImGuiCol_TableRowBgAlt, px.IsTerminated() ? red : green);
						popCount = 2;
					}
				}

				TableSetColumnIndex(0);
				str.Format("%ws", p->GetImageName().c_str());
				Selectable(str, false, ImGuiSelectableFlags_SpanAllColumns);
				//SameLine();
				//TextUnformatted(str);

				::StringCchPrintfA(buffer, sizeof(buffer), "##%d", i);

				if (IsItemClicked()) {
					_selectedProcess = p;
				}

				if (!_modalOpen) {
					if (BeginPopupContextItem(buffer)) {
						_selectedProcess = p;
						if (MenuItem("Kill")) {
							_modalOpen = true;
						}
						EndPopup();
					}
				}
				if (_modalOpen && _selectedProcess == p) {
					TryKillProcess(_selectedProcess.get());
				}

				TableSetColumnIndex(1);
				Text("%6u (0x%05X)", p->Id, p->Id);

				if (TableSetColumnIndex(2)) {
					Text("%4u", p->SessionId);
				}

				if (TableSetColumnIndex(3))
					if (p->CPU > 0 && !px.IsTerminated()) {
						auto value = p->CPU / 10000.0f;
						str.Format("%7.2f  ", value);
						if (p->Id && value > 1.0f) {
							auto color = ImColor::HSV((100 - value) * 50 / 100 / 255.0f, .7f, .4f);
							PushStyleColor(ImGuiCol_ChildBg, color.Value);
							auto size = CalcTextSize(str);
							ATLTRACE(L"%s: %7.2f\n", p->GetImageName().c_str(), value);

							CStringA id;
							id.Format("cpu%d", i);
							BeginChild(id, size, false, ImGuiWindowFlags_None);
							TextColored(ImVec4(1, 1, 1, 1), str);
							EndChild();
							PopStyleColor();
						}
						else {
							TextUnformatted(str);
						}
					}

				if (TableSetColumnIndex(4)) {
					if (p->ParentId > 0) {
						auto parent = _pm.GetProcessById(p->ParentId);
						if (parent && parent->CreateTime < p->CreateTime) {
							Text("%6d (%ws)", parent->Id, parent->GetImageName().c_str());
						}
						else {
							Text("%6d", p->ParentId);
						}
					}
				}

				if (TableSetColumnIndex(5))
					TextUnformatted(W2CA(CTime(*(FILETIME*)&p->CreateTime).Format(L"%x %X")));

				if (TableSetColumnIndex(6))
					Text("%8u K", p->PrivatePageCount >> 10);

				if (TableSetColumnIndex(7))
					Text("%5d", p->BasePriority);
			}
		}
		if (special)
			PopStyleColor(popCount);
		EndTable();
	}
}


void ProcessesView::BuildOptionsMenu() {
	if (BeginMenu("Options")) {
		static bool alwaysOnTop;
		if (MenuItem("Always On Top", nullptr, &alwaysOnTop)) {
			::SetWindowPos(_hMainWnd, !alwaysOnTop ? HWND_NOTOPMOST : HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
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

void ProcessesView::BuildViewMenu() {
	if (BeginMenu("View")) {
		if (BeginMenu("Update Interval")) {
			if (MenuItem("500 ms", nullptr, _updateInterval == 500))
				_updateInterval = 500;
			if (MenuItem("1 second", nullptr, _updateInterval == 1000))
				_updateInterval = 1000;
			if (MenuItem("2 seconds", nullptr, _updateInterval == 2000))
				_updateInterval = 2000;
			if (MenuItem("5 seconds", nullptr, _updateInterval == 5000))
				_updateInterval = 2000;
			Separator();
			if (MenuItem("Paused", "SPACE", _updateInterval == 0))
				_updateInterval = 0;
			ImGui::EndMenu();
		}
		ImGui::EndMenu();
	}
}

void ProcessesView::BuildFileMenu() {
	if (BeginMenu("File")) {
		if (MenuItem("Exit", nullptr, nullptr)) {
			::PostQuitMessage(0);
		}
		ImGui::EndMenu();
	}
}

void ProcessesView::BuildProcessMenu() {
	if (BeginMenu("Process")) {
		if (MenuItem("Kill", "Delete", false, _selectedProcess != nullptr)) {
			TryKillProcess(_selectedProcess.get());
		}
		ImGui::EndMenu();
	}
}


