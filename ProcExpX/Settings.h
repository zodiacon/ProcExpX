#pragma once

#include "ProcessColor.h"


struct Settings {
	Settings();

	enum ProcessColorIndex {
		NewObjects,
		DeletedObjects,
		Manageed,
		Immersive,
		Services,
		Protected,
		Secure,
		InJob,
	};

	std::vector<ProcessColor> ProcessColors;
};

