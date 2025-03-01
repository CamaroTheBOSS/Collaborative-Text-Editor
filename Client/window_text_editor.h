#pragma once
#include "window_base.h"

class TextEditorWindow : public BaseWindow {
	friend class SyncTester;
public:
	TextEditorWindow(const ScrollableScreenBufferBuilder& ssbBuilder);
	Event processChar(TCPClient& client, const KeyPack& key, const std::string& clipboardData) override;
	void processEvent(const TCPClient& client, const Event& pEvent) override;
	void setAuthToken(const std::string& newAuthToken);
	std::string name() const override {
		return className;
	}
	static constexpr const char* className = windows::text_editor::name;
private:
	// events
	void find(const TCPClient& client, const std::vector<std::string>& args);
	void findReset(const TCPClient& client, const std::vector<std::string>& args);
	void findNext(const TCPClient& client, const std::vector<std::string>& args);
	void replace(const TCPClient& client, const std::vector<std::string>& args);

	EventHandlersMap<TextEditorWindow> eventHandlers;
	std::string authToken;
};

