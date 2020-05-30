#include "pch.h"
#include "ProcessColor.h"

ProcessColor::ProcessColor(const char* name, const ImVec4& defaultColor, const ImVec4& defaultTextColor, bool enabled) : 
	Name(name), 
	DefaultColor(defaultColor), Color(defaultColor), 
	DefaultTextColor(defaultTextColor), TextColor(defaultTextColor),
	Enabled(enabled) {
}
