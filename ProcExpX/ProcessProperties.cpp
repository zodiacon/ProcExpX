#include "pch.h"
#include "ProcessProperties.h"

ProcessProperties::ProcessProperties(std::string name, std::shared_ptr<WinSys::ProcessInfo> pi) 
	: WindowProperties(std::move(name)), _pi(std::move(pi)) {
}

WinSys::ProcessInfo* ProcessProperties::GetProcess() const {
	return _pi.get();
}
