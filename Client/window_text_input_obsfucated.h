#pragma once
#include "window_text_input.h"
#include <functional>

class ObsfucatedTextInputWindow : public TextInputWindow {
public:
	using TextInputHandler = std::function<Event(const TextInputWindow&, const ClientSiteDocument&)>;
	ObsfucatedTextInputWindow(const ScrollableScreenBufferBuilder& ssbBuilder, TextInputHandler onSubmitHandler = funcNoop(),
		TextInputHandler onModifyHandler = funcNoop(), TextInputHandler onDeleteHandler = funcNoop());
	Event onDelete() override;
	Event processChar(TCPClient& client, const KeyPack& key, const std::string& clipboardData) override;
private:
	ClientSiteDocument nonObsfucatedDoc;
};