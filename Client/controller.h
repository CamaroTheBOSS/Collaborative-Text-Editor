#pragma once
#include "tcp_client.h"
#include "repository.h"
#include "terminal.h"

class Controller {
public:
	bool connect(const std::string& ip, const int port);
	bool disconnect();
	int readChar();
	bool processChar(const int key);
	void checkIncomingMessages();
	void render();
private:

	TCPClient client;
	Terminal terminal;
	Repository repo;

};
