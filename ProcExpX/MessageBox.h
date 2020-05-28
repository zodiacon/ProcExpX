#pragma once

enum class MessageBoxResult {
	StillOpen,
	Cancel,
	OK,
	Yes,
	No,
};

enum class MessageBoxButtons {
	OK,
	YesNo,
	OkCancel
};

class SimpleMessageBox {
public:
	static MessageBoxResult ShowModal(const char* title, const char* text, MessageBoxButtons buttons = MessageBoxButtons::OK);
};

