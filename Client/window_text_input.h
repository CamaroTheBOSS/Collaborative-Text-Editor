#pragma once
#include "window_base.h"
#include <functional>

class TextInputWindow : public BaseWindow {
public:
	using TextInputHandler = std::function<Event(const TextInputWindow&, const ClientSiteDocument&)>;
	TextInputWindow(const ScrollableScreenBufferBuilder& ssbBuilder, TextInputHandler onSubmitHandler = funcNoop(),
		TextInputHandler onModifyHandler = funcNoop(), TextInputHandler onDeleteHandler = funcNoop());
	Event onDelete() override;
	Event processChar(TCPClient& client, const KeyPack& key, const std::string& clipboardData) override;
	std::string name() const override;
private:
	static TextInputWindow::TextInputHandler funcNoop();

	const std::string winName = "TextInputWindow";
	TextInputHandler onSubmitHandler;
	TextInputHandler onModifyHandler;
	TextInputHandler onDeleteHandler;
};