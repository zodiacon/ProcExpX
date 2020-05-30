#include "pch.h"
#include "ProcessesView.h"
#include <algorithm>
#include "SortHelper.h"
#include "Processes.h"
#include "TabManager.h"
#include <shellapi.h>
#include "FormatHelper.h"
#include "ImGuiExt.h"
#include "Globals.h"
#include "colors.h"

using namespace ImGui;

ProcessesView::ProcessesView(TabManager& tm) : _tm(tm), _pm(Globals::Get().ProcMgr) {
}

void ProcessesView::BuildWindow() {
	if (BeginMenuBar()) {
		BuildFileMenu();
		BuildViewMenu();
		BuildProcessMenu();
		_tm.BuildOptionsMenu();

		EndMenuBar();
	}

	BuildToolBar();
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
			case 16: return SortHelper::SortNumbers(GetProcessInfoEx(p1.get()).GetAttributes(_pm), GetProcessInfoEx(p2.get()).GetAttributes(_pm), asc);

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
	auto& g = Globals::Get();

	if (BeginTable("processes", 17, ImGuiTableFlags_BordersV | ImGuiTableFlags_Sortable | ImGuiTableFlags_ScrollFreeze2Columns |
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
		TableSetupColumn("Attributes");

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

		int popCount = 3;
		static bool selected = false;

		auto orgBackColor = GetStyle().Colors[ImGuiCol_TableRowBg];

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

				auto colors = px.GetColors(_pm);
				special = colors.first.x >= 0 || p == _selectedProcess;
				if (special) {
					if (p == _selectedProcess) {
						const auto& color = GetStyle().Colors[ImGuiCol_TextSelectedBg];
						PushStyleColor(ImGuiCol_TableRowBg, color);
						PushStyleColor(ImGuiCol_TableRowBgAlt, color);
						PushStyleColor(ImGuiCol_Text, GetStyle().Colors[ImGuiCol_Text]);
					}
					else {
						PushStyleColor(ImGuiCol_TableRowBg, colors.first);
						PushStyleColor(ImGuiCol_TableRowBgAlt, colors.first);
						PushStyleColor(ImGuiCol_Text, colors.second);
					}
				}

				TableSetColumnIndex(0);
				str.Format("%ws##%d", p->GetImageName().c_str(), i);
				Selectable(str, false, ImGuiSelectableFlags_SpanAllColumns);

				::StringCchPrintfA(buffer, sizeof(buffer), "##%d", i);

				if (!_modalOpen && IsItemClicked()) {
					_selectedProcess = p;
				}

				if (!_modalOpen) {
					if (IsKeyPressed(VK_DELETE) && _selectedProcess != nullptr)
						_modalOpen = true;

					if (BeginPopupContextItem(buffer)) {
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
				PushFont(g.MonoFont);
				Text("%6u (0x%05X)", p->Id, p->Id);
				PopFont();

				if (TableSetColumnIndex(2)) {
					Text("%4u", p->SessionId);
				}

				if (TableSetColumnIndex(3)) {
					if (p->CPU > 0 && !px.IsTerminated()) {
						PushFont(g.MonoFont);
						auto value = p->CPU / 10000.0f;
						str.Format("%7.2f  ", value);
						ImVec4 color;
						auto customColors = p->Id && value > 1.0f;
						if (customColors) {
							color = ImColor::HSV((100 - value) * 50 / 100 / 255.0f, .7f, .4f).Value;
						}
						else {
							color = orgBackColor;
						}
						PushStyleColor(ImGuiCol_ChildBg, color);
						auto size = CalcTextSize(str);
						CStringA id;
						id.Format("cpu%d", i);
						BeginChild(id, size, false, ImGuiWindowFlags_None);
						if (customColors) {
							TextColored(ImVec4(1, 1, 1, 1), str);
						}
						else {
							TextUnformatted(str);
						}
						EndChild();
						PopStyleColor();
						PopFont();
					}
				}

				if (TableSetColumnIndex(4)) {
					if (p->ParentId > 0) {
						auto parent = _pm.GetProcessById(p->ParentId);
						if (parent && parent->CreateTime < p->CreateTime) {
							PushFont(g.MonoFont);
							Text("%6d ", parent->Id);
							PopFont();
							SameLine();
							Text("(%ws)", parent->GetImageName().c_str());
						}
						else {
							PushFont(g.MonoFont);
							Text("%6d", p->ParentId);
							PopFont();
						}
					}
				}

				if (TableSetColumnIndex(5))
					TextUnformatted(W2CA(CTime(*(FILETIME*)&p->CreateTime).Format(L"%x %X")));

				if (TableSetColumnIndex(6)) {
					PushFont(g.MonoFont);
					Text("%12s K", FormatHelper::FormatWithCommas(p->PrivatePageCount >> 10));
					PopFont();
				}

				if (TableSetColumnIndex(7)) {
					PushFont(g.MonoFont);
					Text("%5d", p->BasePriority);
					PopFont();
				}
				if (TableSetColumnIndex(8)) {
					PushFont(g.MonoFont);
					Text("%6d", p->ThreadCount);
					PopFont();
				}
				if (TableSetColumnIndex(9)) {
					PushFont(g.MonoFont);
					Text("%6d", p->HandleCount);
					PopFont();
				}
				if (TableSetColumnIndex(10)) {
					PushFont(g.MonoFont);
					Text("%12s K", FormatHelper::FormatWithCommas(p->WorkingSetSize >> 10));
					PopFont();
				}
				if (TableSetColumnIndex(11))
					Text("%ws", px.GetExecutablePath().c_str());

				if (TableSetColumnIndex(12)) {
					PushFont(g.MonoFont);
					auto total = p->UserTime + p->KernelTime;
					Text("%ws", (PCWSTR)FormatHelper::TimeSpanToString(total));
					PopFont();
				}

				if (TableSetColumnIndex(13)) {
					PushFont(g.MonoFont);
					Text("%6d", p->PeakThreads);
					PopFont();
				}

				if (TableSetColumnIndex(14)) {
					PushFont(g.MonoFont);
					Text("%14s K", FormatHelper::FormatWithCommas(p->VirtualSize >> 10));
					PopFont();
				}

				if (TableSetColumnIndex(15)) {
					PushFont(g.MonoFont);
					Text("%12s K", FormatHelper::FormatWithCommas(p->PeakWorkingSetSize >> 10));
					PopFont();
				}

				if (TableSetColumnIndex(16))
					TextUnformatted(ProcessAttributesToString(px.GetAttributes(_pm)));
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
		//Separator();
		ImGui::EndMenu();
	}
}

void ProcessesView::BuildToolBar() {
	Separator();
	if (ButtonEnabled("Kill", _selectedProcess != nullptr)) {
		bool success;
		TryKillProcess(_selectedProcess.get(), success);
	}
	SameLine();
	bool open = Button("Colors");
	if (open)
		OpenPopup("colors");

	if (BeginPopup("colors", ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
		auto& colors = Globals::Get().GetSettings().ProcessColors;

		for (auto& c : colors) {
			Checkbox(c.Name, &c.Enabled);
			SameLine(150);
			ColorEdit4("Background##" + c.Name, (float*)&c.Color, ImGuiColorEditFlags_NoInputs);
			SameLine();
			if (Button("Reset##" + c.Name))
				c.Color = c.DefaultColor;

			SameLine();
			ColorEdit4("Text##" + c.Name, (float*)&c.TextColor, ImGuiColorEditFlags_NoInputs);
			SameLine();
			if (Button("Reset##Text" + c.Name))
				c.TextColor = c.DefaultTextColor;
		}

		EndPopup();
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

CStringA ProcessesView::ProcessAttributesToString(ProcessAttributes attributes) {
	CStringA text;

	static const struct {
		ProcessAttributes Attribute;
		const char* Text;
	} attribs[] = {
		{ ProcessAttributes::Managed, "Managed" },
		{ ProcessAttributes::Immersive, "Immersive" },
		{ ProcessAttributes::Protected, "Protected" },
		{ ProcessAttributes::Secure, "Secure" },
		{ ProcessAttributes::Service, "Service" },
		{ ProcessAttributes::InJob, "In Job" },
	};

	for (auto& item : attribs)
		if ((item.Attribute & attributes) == item.Attribute)
			text += CStringA(item.Text) + ", ";
	if (!text.IsEmpty())
		text = text.Mid(0, text.GetLength() - 2);
	return text;
}


