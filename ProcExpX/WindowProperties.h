#pragma once

class WindowProperties {
public:
	WindowProperties(std::string name);
	const std::string& GetName() const;

	bool WindowOpen = true;

private:
	std::string _name;
};

