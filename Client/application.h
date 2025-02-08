#pragma once
#include "window_base.h"
#include "terminal.h"
#include "repository.h"

#include <unordered_map>

class Application {
	friend class SyncTester;
public:
	Application();
	bool connect(const std::string& ip, const int port);
	bool disconnect();
	bool isConnected() const;
	KeyPack readChar() const;
	ActionDone processChar(const KeyPack& key);
	bool checkIncomingMessages();
	bool checkBufferWasResized();
	bool requestDocument(const std::chrono::milliseconds& timeout, const int tries);
	void render();
private:
	void setFocus(const int windowIdx);
	void showSearchWindow();
	void destroyLastWindow();
	void changeFocusUp();
	void changeFocusDown();
	TCPClient client;
	Terminal terminal;
	client::Repository repo;

	int focus = 0;
	std::vector<std::unique_ptr<BaseWindow>> windows;
	std::unordered_map<std::string, bool> windowsRegistry;
	
	
};
