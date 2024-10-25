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
		return wsaError;
	}

	Controller controller{};
	if (!controller.connect("192.168.1.10", 8081)) {
		std::cout << "Connection to server failed!\n";
		return -1;
	}
	if (!controller.requestDocument(std::chrono::milliseconds(500), 3)) {
		std::cout << "Requesting document from the server failed!\n";
		return -1;
	}
	while (true) {
		int key = controller.readChar();
		controller.processChar(key);
		bool render = controller.checkIncomingMessages();
		if (render) {
			controller.render();
		}
	}

	controller.disconnect();
	WSACleanup();

	return 0;
}