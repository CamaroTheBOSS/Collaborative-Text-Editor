#pragma once
#include "tcp_client.h"
#include "repository.h"
#include "terminal.h"

class Controller {
	friend class SyncTester;
public:
	bool connect(const std::string& ip, const int port);
	bool disconnect();
	bool isConnected() const;
	KeyPack readChar() const;
	bool processChar(const KeyPack& key);
	bool checkIncomingMessages();
	bool requestDocument(const std::chrono::milliseconds& timeout, const int tries);
	void render();
	bool saveDoc() const;

private:

	TCPClient client;
	Terminal terminal;
	client::Repository repo;
};
