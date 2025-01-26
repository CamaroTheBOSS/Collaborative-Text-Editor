#include "test_server.h"

#include <iostream>

TestServer::TestServer(const std::string& ip, const int port) :
	server(ip, port) {}

void TestServer::start() {
	serverThread = std::thread{ &TestServer::run, this };
}

void TestServer::stop() {
	server.close();
	if (serverThread.joinable()) {
		serverThread.join();
	}
}

void TestServer::run() {
	if (!server.open(4)) {
		std::cout << "Error when opening server\n";
		return;
	}
	server.start();
}