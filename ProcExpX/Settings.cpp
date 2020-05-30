#include "pch.h"
#include "Settings.h"
#include "colors.h"
#include "FormatHelper.h"

using namespace ImGui;

Settings::Settings() {
	auto& black = StandardColors::Black;
	auto& white = StandardColors::White;

	ProcessColors = {
		ProcessColor("New Objects", FormatHelper::ColorWithAlpha(StandardColors::DarkGreen, .7f), white),
		ProcessColor("Deleted Objects", FormatHelper::ColorWithAlpha(StandardColors::DarkRed, .7f), white),
		ProcessColor("Managed (.NET)", FormatHelper::ColorWithAlpha(StandardColors::Yellow, .7f), black),
		ProcessColor("Immersive", FormatHelper::ColorWithAlpha(StandardColors::Cyan, .7f), black),
		ProcessColor("Services", FormatHelper::ColorWithAlpha(StandardColors::Pink, .7f), black),
		ProcessColor("Protected", FormatHelper::ColorWithAlpha(StandardColors::Fuchsia, .7f), white),
		ProcessColor("Secure", FormatHelper::ColorWithAlpha(StandardColors::Purple, .7f), white),
		ProcessColor("In Job", FormatHelper::ColorWithAlpha(StandardColors::Brown, .7f), white, false),
	};
}
