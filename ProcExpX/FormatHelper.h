#pragma once

#include "imgui.h"

struct FormatHelper {
	static ATL::CString TimeSpanToString(int64_t ts);
	static CStringA FormatWithCommas(long long size);
	static ImVec4 ColorWithAlpha(const ImVec4& color, float alpha);
};

