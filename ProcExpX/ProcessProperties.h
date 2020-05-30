#pragma once

#include "ProcessInfo.h"
#include "WindowProperties.h"

class ProcessProperties : public WindowProperties {
public:
	ProcessProperties(std::string name, std::shared_ptr<WinSys::ProcessInfo> pi);
	WinSys::ProcessInfo* GetProcess() const;

private:
	std::shared_ptr<WinSys::ProcessInfo> _pi;
};

