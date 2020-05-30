#include "pch.h"
#include "WindowProperties.h"

WindowProperties::WindowProperties(std::string name) : _name(std::move(name)) {
}

const std::string& WindowProperties::GetName() const {
	return _name;
}
