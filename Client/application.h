#pragma once
#include "controller_base.h"
#include "terminal.h"

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
	std::unique_ptr<BaseController> controller;
	client::Repository repo;
	
};
