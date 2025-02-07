#pragma once
#include "window_base.h"
#include "terminal.h"
#include "repository.h"

#include <list>

class Application {
	friend class SyncTester;
public:
	Application();
	bool connect(const std::string& ip, const int port);
	bool disconnect();
	bool isConnected() const;
	KeyPack readChar() const;
	bool processChar(const KeyPack& key);
	bool checkIncomingMessages();
	bool requestDocument(const std::chrono::milliseconds& timeout, const int tries);
	void render();
private:
	TCPClient client;
	Terminal terminal;
	int focus = 0;
	std::vector<std::unique_ptr<BaseWindow>> windows;
	client::Repository repo;
	
};
