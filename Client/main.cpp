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

	Application app{ "192.168.1.10", 8081 };
	app.render();
	auto actionDone = false;
	bool render = false;
	while (true) {
		KeyPack key = app.readChar();
		actionDone = key.keyCode != '\0' ? app.processChar(key) : false;
		render = app.checkIncomingMessages();
		if (render || actionDone) {
			app.render();
		}
	}
	WSACleanup();

	return 0;
}