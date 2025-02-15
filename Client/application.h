#pragma once
#include "window_base.h"
#include "terminal.h"
#include "repository.h"

#include <unordered_map>
using Windows = std::vector<std::unique_ptr<BaseWindow>>;
using WindowsRegistry = std::unordered_map<std::string, bool>;
using WindowsIt = Windows::const_iterator;
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
	WindowsIt showSearchWindow();
	WindowsIt showReplaceWindow();
	WindowsIt findWindow(const std::string& name) const;
	void destroyLastWindow();
	void changeFocusUp();
	void changeFocusDown();
	TCPClient client;
	Terminal terminal;
	client::Repository repo;

	int focus = 0;
	Windows windows;
	WindowsRegistry windowsRegistry;
	
	
};
