#pragma once

#include <memory>
#include <ProcessInfo.h>
#include "imgui.h"

class ProcessInfoEx {
public:
	ProcessInfoEx(WinSys::ProcessInfo* pi) : _pi(pi) {}

	bool IsNew() const {
		return _isNew;
	}

	bool IsTerminated() const {
		return _isTerminated;
	}

	ImVec4 GetColor() const;

	bool Update();
	void New(uint32_t ms);
	void Term(uint32_t ms);
	const std::wstring& GetExecutablePath() const;

private:
	DWORD64 _expiryTime;
	WinSys::ProcessInfo* _pi;
	mutable std::wstring _executablePath;
	bool _isNew : 1 = false, _isTerminated : 1 = false;
};

