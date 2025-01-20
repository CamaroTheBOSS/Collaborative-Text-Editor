#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")

#include <iostream>
#include <chrono>

#include "controller.h";



int main() {
	WSADATA wsaData;
	WORD mVersionRequested = MAKEWORD(2, 2);
	int wsaError = WSAStartup(mVersionRequested, &wsaData);
	if (wsaError) {
		std::cout << wsaError << " Error on WSA stratup\n";
		WSACleanup();
		return 0;
	}

	Controller controller;
	if (!controller.connect("192.168.1.10", 8081)) {
		std::cout << "Connection to server failed!\n";
		return 0;
	}
	if (!controller.requestDocument(std::chrono::milliseconds(50000), 3)) {
		std::cout << "Requesting document from the server failed!\n";
		return 0;
	}
	controller.render();
	while (controller.isConnected()) {
		KeyPack key = controller.readChar();
		controller.processChar(key);
		bool render = controller.checkIncomingMessages();
		if (render) {
			controller.render();
		}
	}
	controller.saveDoc();
	WSACleanup();

	return 0;
}