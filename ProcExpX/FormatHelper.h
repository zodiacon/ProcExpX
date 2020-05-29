#pragma once

#include <atlstr.h>
#include <stdint.h>

struct FormatHelper {
	static ATL::CString TimeSpanToString(int64_t ts);
	static CStringA FormatWithCommas(long long size);
};

