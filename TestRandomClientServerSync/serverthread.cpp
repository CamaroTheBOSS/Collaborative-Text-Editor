#include "server.h"
#include "serverthread.h"
#include <iostream>

void serverThread() {
	Server server{ "192.168.1.10", 8081 };
	if (!server.open(4)) {
		std::cout << "Error when opening server\n";
		return;
	}
	server.start();
	server.close();
}