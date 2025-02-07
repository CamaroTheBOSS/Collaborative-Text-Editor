#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")

#include <iostream>
#include <chrono>

#include "application.h";



int main() {
	WSADATA wsaData;
	WORD mVersionRequested = MAKEWORD(2, 2);
	int wsaError = WSAStartup(mVersionRequested, &wsaData);
	if (wsaError) {
		std::cout << wsaError << " Error on WSA stratup\n";
		WSACleanup();
		return 0;
	}

	Application app;
	if (!app.connect("192.168.1.10", 8081)) {
		std::cout << "Connection to server failed!\n";
		return 0;
	}
	if (!app.requestDocument(std::chrono::milliseconds(500), 3)) {
		std::cout << "Requesting document from the server failed!\n";
		return 0;
	}
	app.render();
	while (app.isConnected()) {
		KeyPack key = app.readChar();
		app.processChar(key);
		bool render = app.checkIncomingMessages();
		if (render) {
			app.render();
		}
	}
	WSACleanup();

	return 0;
}