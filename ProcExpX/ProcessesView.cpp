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
#include "TabManager.h"
#include <shellapi.h>
#include "FormatHelper.h"

using namespace ImGui;

ProcessesView::ProcessesView(TabManager& tm) : _tm(tm) {
}

void ProcessesView::BuildWindow() {
	if (BeginMenuBar()) {
		BuildFileMenu();
		BuildViewMenu();
		BuildProcessMenu();
		_tm.BuildOptionsMenu();

		EndMenuBar();
	}

	BuildTable();
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
			case 8: return SortHelper::SortNumbers(p1->ThreadCount, p2->ThreadCount, asc);
			case 9: return SortHelper::SortNumbers(p1->HandleCount, p2->HandleCount, asc);
			case 10: return SortHelper::SortNumbers(p1->WorkingSetSize, p2->WorkingSetSize, asc);
			case 11: return SortHelper::SortStrings(GetProcessInfoEx(p1.get()).GetExecutablePath(), GetProcessInfoEx(p2.get()).GetExecutablePath(), asc);
			case 12: return SortHelper::SortNumbers(p1->KernelTime + p1->UserTime, p2->KernelTime + p2->UserTime, asc);
			case 13: return SortHelper::SortNumbers(p1->PeakThreads, p2->PeakThreads, asc);
			case 14: return SortHelper::SortNumbers(p1->VirtualSize, p2->VirtualSize, asc);
			case 15: return SortHelper::SortNumbers(p1->PeakWorkingSetSize, p2->PeakWorkingSetSize, asc);

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
		px.New(2000);
	}

	for (auto& pi : _pm.GetTerminatedProcesses()) {
		auto& px = GetProcessInfoEx(pi.get());
		px.Term(2000);
	}
}

bool ProcessesView::KillProcess(uint32_t id) {
	auto process = WinSys::Process::OpenById(id, WinSys::ProcessAccessMask::Terminate);
	if (process == nullptr)
		return false;

	return process->Terminate();
}

bool ProcessesView::TryKillProcess(WinSys::ProcessInfo* pi, bool& success) {
	_modalOpen = true;
	CStringA text;
	text.Format("Kill process %u (%ws)?",
		_selectedProcess->Id, _selectedProcess->GetImageName().c_str());

	auto result = SimpleMessageBox::ShowModal("Kill Process?", text, MessageBoxButtons::OkCancel);
	if (result != MessageBoxResult::StillOpen) {
		_modalOpen = false;
		if (result == MessageBoxResult::OK) {
			success = KillProcess(_selectedProcess->Id);
			if (success)
				_selectedProcess.reset();
		}
		return true;
	}
	return false;
}

void ProcessesView::BuildTable() {
	if (BeginTable("processes", 16, ImGuiTableFlags_BordersV | ImGuiTableFlags_Sortable | ImGuiTableFlags_ScrollFreeze2Columns |
		ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollFreezeTopRow | ImGuiTableFlags_Reorderable | ImGuiTableFlags_BordersVFullHeight |
		ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Scroll | ImGuiTableFlags_RowBg)) {

		TableSetupColumn("Name", ImGuiTableColumnFlags_None);
		TableSetupColumn("Id");
		TableSetupColumn("Session");
		TableSetupColumn("CPU (%)");
		TableSetupColumn("Parent Process", ImGuiTableColumnFlags_None, 180);
		TableSetupColumn("Created", ImGuiTableColumnFlags_WidthFixed);
		TableSetupColumn("Private Bytes");
		TableSetupColumn("Priority");
		TableSetupColumn("Threads");
		TableSetupColumn("Handles");
		TableSetupColumn("Working Set");
		TableSetupColumn("Executable Path", ImGuiTableColumnFlags_None, 200);
		TableSetupColumn("CPU Time");
		TableSetupColumn("Peak Threads");
		TableSetupColumn("Virtual Size");
		TableSetupColumn("Peak Working Set");

		TableAutoHeaders();

		if (IsKeyPressed(VK_SPACE)) {
			TogglePause();
		}

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
		static char buffer[64];
		CStringA str;

		int popCount;
		static bool selected = false;

		if (_killFailed) {
			if (MessageBoxResult::StillOpen != SimpleMessageBox::ShowModal("Kill Process", "Failed to kill process!"))
				_killFailed = false;
		}

		while (clipper.Step()) {
			for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
				auto& p = _processes[i];
				auto& px = GetProcessInfoEx(p.get());

				TableNextRow();

				if (special)
					PopStyleColor(popCount);

				auto color = ImVec4(0, 0, 1, .8f);// GetStyle().Colors[ImGuiCol_TextSelectedBg];

				special = px.IsNew() || px.IsTerminated() || p == _selectedProcess;
				if (special) {
					if (p == _selectedProcess) {
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
				PushStyleColor(ImGuiCol_TextSelectedBg, ImVec4(1, 1, 0, .5f));

				Selectable(str, false, ImGuiSelectableFlags_SpanAllColumns);
				PopStyleColor();

				::StringCchPrintfA(buffer, sizeof(buffer), "##%d", i);

				if (!_modalOpen && IsItemClicked()) {
					_selectedProcess = p;
				}

				if (!_modalOpen) {
					if (IsKeyPressed(VK_DELETE) && _selectedProcess != nullptr)
						_modalOpen = true;

					if (BeginPopupContextItem(buffer)) {
						if (special) {
							PopStyleColor();
							popCount--;
						}
						_selectedProcess = p;
						BuildPriorityClassMenu(p.get());
						Separator();
						if (MenuItem("Kill")) {
							_modalOpen = true;
						}
						Separator();
						if (MenuItem("Go to file location")) {
							GotoFileLocation(_selectedProcess.get());
						}
						if (special) {
							PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
							popCount++;
						}
						EndPopup();
					}
				}

				if (_modalOpen && _selectedProcess == p) {
					bool success;
					if (TryKillProcess(_selectedProcess.get(), success)) {
						if (!success)
							_killFailed = true;
					}
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
					Text("%12s K", FormatHelper::FormatWithCommas(p->PrivatePageCount >> 10));

				if (TableSetColumnIndex(7))
					Text("%5d", p->BasePriority);

				if (TableSetColumnIndex(8))
					Text("%6d", p->ThreadCount);

				if (TableSetColumnIndex(9))
					Text("%6d", p->HandleCount);

				if (TableSetColumnIndex(10))
					Text("%12s K", FormatHelper::FormatWithCommas(p->WorkingSetSize >> 10));

				if (TableSetColumnIndex(11))
					Text("%ws", px.GetExecutablePath().c_str());

				if (TableSetColumnIndex(12)) {
					auto total = p->UserTime + p->KernelTime;
					Text("%ws", (PCWSTR)FormatHelper::TimeSpanToString(total));
				}

				if (TableSetColumnIndex(13)) {
					Text("%6d", p->PeakThreads);
				}

				if (TableSetColumnIndex(14))
					Text("%14s K", FormatHelper::FormatWithCommas(p->VirtualSize >> 10));

				if (TableSetColumnIndex(15))
					Text("%12s K", FormatHelper::FormatWithCommas(p->PeakWorkingSetSize >> 10));
			}
		}
		if (special)
			PopStyleColor(popCount);
		EndTable();
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
				TogglePause();
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
		if (_selectedProcess) {
			BuildPriorityClassMenu(_selectedProcess.get());
			Separator();
		}
		if (MenuItem("Kill", "Delete", false, _selectedProcess != nullptr)) {
			bool success;
			if (TryKillProcess(_selectedProcess.get(), success) && !success)
				_killFailed = true;
		}
		Separator();
		ImGui::EndMenu();
	}
}

void ProcessesView::BuildPriorityClassMenu(WinSys::ProcessInfo* pi) {
	using namespace WinSys;

	auto process = Process::OpenById(pi->Id,
		ProcessAccessMask::QueryLimitedInformation | ProcessAccessMask::SetInformation);
	bool enabled = process != nullptr;
	if (!enabled)
		process = Process::OpenById(pi->Id, ProcessAccessMask::QueryLimitedInformation);

	ProcessPriorityClass pc;
	if (process)
		pc = process->GetPriorityClass();

	if (BeginMenu("Priority")) {
		if (MenuItem("Idle (4)", nullptr, pc == ProcessPriorityClass::Idle, enabled && pc != ProcessPriorityClass::Idle))
			process->SetPriorityClass(ProcessPriorityClass::Idle);
		if (MenuItem("Below Normal (6)", nullptr, pc == ProcessPriorityClass::BelowNormal, enabled && pc != ProcessPriorityClass::BelowNormal))
			process->SetPriorityClass(ProcessPriorityClass::BelowNormal);
		if (MenuItem("Normal (8)", nullptr, pc == ProcessPriorityClass::Normal, enabled && pc != ProcessPriorityClass::Normal))
			process->SetPriorityClass(ProcessPriorityClass::BelowNormal);
		if (MenuItem("Above Normal (10)", nullptr, pc == ProcessPriorityClass::AboveNormal, enabled && pc != ProcessPriorityClass::AboveNormal))
			process->SetPriorityClass(ProcessPriorityClass::AboveNormal);
		if (MenuItem("High (13)", nullptr, pc == ProcessPriorityClass::High, enabled && pc != ProcessPriorityClass::High))
			process->SetPriorityClass(ProcessPriorityClass::High);
		if (MenuItem("Real-time (24)", nullptr, pc == ProcessPriorityClass::Realtime, enabled && pc != ProcessPriorityClass::Realtime))
			process->SetPriorityClass(ProcessPriorityClass::Realtime);

		ImGui::EndMenu();
	}
}

bool ProcessesView::GotoFileLocation(WinSys::ProcessInfo* pi) {
	ATLASSERT(pi);
	auto& px = GetProcessInfoEx(pi);
	auto& path = px.GetExecutablePath();
	auto bs = path.rfind(L'\\');
	if (bs == std::wstring::npos)
		return false;

	auto folder = path.substr(0, bs);
	return (INT_PTR)::ShellExecute(nullptr, L"open", L"explorer", (L"/select,\"" + path + L"\"").c_str(),
		nullptr, SW_SHOWDEFAULT) > 31;
}

void ProcessesView::TogglePause() {
	if (_updateInterval == 0)
		_updateInterval = _oldInterval;
	else {
		_oldInterval = _updateInterval;
		_updateInterval = 0;
	}
}


