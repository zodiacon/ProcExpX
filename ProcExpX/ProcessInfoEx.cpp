#include <Windows.h>
#include "ProcessInfoEx.h"
#include "Processes.h"
#include "colors.h"


std::pair<const ImVec4&, const ImVec4&> ProcessInfoEx::GetColors() const {
	using namespace ImGui;
	static const ImVec4 red(.8f, 0, 0, .8f), green(0, .6f, 0, .8f);
	static const ImVec4 white(1, 1, 1, 1), black(0, 0, 0, 1);

	if (IsTerminated())
		return { red, white };
	if (IsNew())
		return { green, white };

	if (_color.w == 0) {
		auto attributes = GetAttributes();
		auto alpha = .7f;
		if ((attributes & ProcessAttributes::Managed) == ProcessAttributes::Managed) {
			_color = ImVec4(1, 1, 0, alpha);
			_textColor = black;
		}
		else if ((attributes & ProcessAttributes::Protected) == ProcessAttributes::Protected) {
			_color = ImVec4(.7f, 0, .7f, alpha);
			_textColor = white;
		}
		else if ((attributes & ProcessAttributes::Immersive) == ProcessAttributes::Immersive) {
			_color = StandardColors::Cyan; //ImVec4(.5, .7f, .9f, alpha);
			_color.w = alpha;
			_textColor = black;
		}
		else if ((attributes & ProcessAttributes::Secure) == ProcessAttributes::Secure) {
			_color = ImVec4(.6f, .2f, .7f, alpha);
			_textColor = black;
		}
		//else if ((attributes & ProcessAttributes::InJob) == ProcessAttributes::InJob) {
		//	_color = ImVec4(.5f, .7f, .2f, alpha);
		//	_textColor = white;
		//}
	}
	if (_color.w == 0) {
		_color = ImVec4(-1, 0, 0, 0);
	}

	return { _color, _textColor };
}

ProcessAttributes ProcessInfoEx::GetAttributes() const {
	if (_attributes == ProcessAttributes::NotComputed) {
		_attributes = ProcessAttributes::None;
		auto process = WinSys::Process::OpenById(_pi->Id, WinSys::ProcessAccessMask::QueryLimitedInformation);
		if (process) {
			if (process->IsManaged())
				_attributes |= ProcessAttributes::Managed;
			if (process->IsProtected())
				_attributes |= ProcessAttributes::Protected;
			if (process->IsImmersive())
				_attributes |= ProcessAttributes::Immersive;
			if (process->IsSecure())
				_attributes |= ProcessAttributes::Secure;
			if (process->IsInJob())
				_attributes |= ProcessAttributes::InJob;
		}
	}
	return _attributes;
}

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

const std::wstring& ProcessInfoEx::GetExecutablePath() const {
	if (_executablePath.empty() && _pi->Id != 0) {
		const auto& path = _pi->GetNativeImagePath();
		if (path[0] == L'\\') {
			auto process = WinSys::Process::OpenById(_pi->Id, WinSys::ProcessAccessMask::QueryLimitedInformation);
			if (process)
				_executablePath = process->GetFullImageName();
		}
		else {
			_executablePath = path;
		}
	}
	return _executablePath;
}
