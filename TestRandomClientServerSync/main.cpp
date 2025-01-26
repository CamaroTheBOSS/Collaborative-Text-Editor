#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")

#include "sync_tester.h"
#include "args.h"
#include <iostream>

int main(int argc, char* argv[]) {
	WSADATA wsaData;
	WORD mVersionRequested = MAKEWORD(2, 2);
	int wsaError = WSAStartup(mVersionRequested, &wsaData);
	if (wsaError) {
		std::cout << wsaError << " Error on WSA stratup\n";
		WSACleanup();
		return -1;
	}
	Args args{ argc, argv };
	if (!args.isValid()) {
		return 0;
	}

	SyncTester tester{ "192.168.1.10", 8081, args["seed"]};
	tester.start();
	WSACleanup();
	return 0;
}