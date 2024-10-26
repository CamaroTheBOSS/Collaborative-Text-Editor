#pragma once
#include "tcp_client.h"
#include "repository.h"
#include "terminal.h"

class Controller {
public:
	bool connect(const std::string& ip, const int port);
	bool disconnect();
	bool isConnected() const;
	int readChar() const;
	bool processChar(const int key);
	bool checkIncomingMessages();
	bool requestDocument(const std::chrono::milliseconds& timeout, const int tries);
	void render();

private:

	TCPClient client;
	Terminal terminal;
	Repository repo;
};
