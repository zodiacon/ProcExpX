#pragma once

#include <memory>
#include <ProcessInfo.h>
#include "imgui.h"

enum class ProcessAttributes {
	NotComputed = -1,
	None = 0,
	Protected = 1,
	InJob = 2,
	Service = 4,
	Managed = 8,
	Secure = 0x10,
	Immersive = 0x20,
};
DEFINE_ENUM_FLAG_OPERATORS(ProcessAttributes);

class ProcessInfoEx {
public:
	ProcessInfoEx(WinSys::ProcessInfo* pi) : _pi(pi) {}

	bool IsNew() const {
		return _isNew;
	}

	bool IsTerminated() const {
		return _isTerminated;
	}

	std::pair<const ImVec4&, const ImVec4&> GetColors() const;
	ProcessAttributes GetAttributes() const;

	bool Update();
	void New(uint32_t ms);
	void Term(uint32_t ms);
	const std::wstring& GetExecutablePath() const;

private:
	DWORD64 _expiryTime;
	WinSys::ProcessInfo* _pi;
	mutable std::wstring _executablePath;
	mutable ImVec4 _color{}, _textColor{};
	mutable ProcessAttributes _attributes = ProcessAttributes::NotComputed;
	mutable 
	bool _isNew : 1 = false, _isTerminated : 1 = false;
};

