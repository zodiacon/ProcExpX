#pragma once

#include <memory>
#include <ProcessInfo.h>

class ProcessInfoEx {
public:
	ProcessInfoEx(WinSys::ProcessInfo* pi) : _pi(pi) {}

	bool IsNew() const {
		return _isNew;
	}

	bool IsTerminated() const {
		return _isTerminated;
	}
	bool Update();
	void New(uint32_t ms);
	void Term(uint32_t ms);

private:
	DWORD64 _expiryTime;
	WinSys::ProcessInfo* _pi;
	bool _isNew : 1 = false, _isTerminated : 1 = false;
};

