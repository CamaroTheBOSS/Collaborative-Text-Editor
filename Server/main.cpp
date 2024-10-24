#include <WinSock2.h>
#include <iostream>

#include "server.h"

#pragma comment(lib, "Ws2_32.lib")


int main() {
	WSADATA wsaData;
	WORD mVersionRequested = MAKEWORD(2, 2);
	int wsaError = WSAStartup(mVersionRequested, &wsaData);
	if (wsaError) {
		std::cout << wsaError << " Error on WSA stratup\n";
		WSACleanup();
		return wsaError;
	}

	Server server{ "192.168.1.10", 8081};
	if (!server.open(4)) {
		std::cout << " Error when opening server\n";
		return -1;
	}
	server.start();
	server.close();

	WSACleanup();
	return 0;
}