#include <Windows.h>
#include "ProcessInfoEx.h"

bool ProcessInfoEx::Update() {
	if (!_isNew && !_isTerminated)
		return false;

	bool term = _isTerminated;
	if (::GetTickCount() > _expiryTime) {
		_isTerminated = _isNew = false;
		return term;
	}
	return false;
}

void ProcessInfoEx::New(uint32_t ms) {
	_isNew = true;
	_expiryTime = ::GetTickCount64() + ms;
}

void ProcessInfoEx::Term(uint32_t ms) {
	_isNew = false;
	_isTerminated = true;
	_expiryTime = ::GetTickCount64() + ms;
}
