#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")

#include <iostream>
#include <chrono>
#include <random>

#include "application.h";

auto getRandomEngine() {
	std::random_device device;
	std::random_device::result_type data[(std::mt19937::state_size - 1) / sizeof(device()) + 1];
	std::generate(std::begin(data), std::end(data), std::ref(device));
	std::seed_seq seed{std::begin(data), std::end(data)};
	return std::mt19937(seed);
}

static auto randomEngine = getRandomEngine();

int getRandomKey() {
	std::uniform_int_distribution<> actionDist(0, 12);
	std::uniform_int_distribution<> charDist(32, 127);

	int action = actionDist(randomEngine);
	int key = 0;
	if (action < 5) {
		key = charDist(randomEngine);
	}
	else if (action < 7) {
		key = BACKSPACE;
	}
	else if (action < 9) {
		key = ARROW_LEFT;
	}
	else if (action < 10) {
		key = ARROW_RIGHT;
	}
	else if (action < 11) {
		key = ARROW_DOWN;
	}
	else {
		key = ARROW_UP;
	}
	return key;
}


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
	app.processChar(KeyPack{ENTER, false});
	app.processChar(KeyPack{ 's', false});
	app.processChar(KeyPack{ ENTER, false });
	app.render();
	while (app.isConnected()) {
		KeyPack key = app.readChar();
		if (key.keyCode == '\0') {
			key.keyCode = getRandomKey();
		}
		app.processChar(key);
		bool render = app.checkIncomingMessages();
		if (render) {
			app.render();
		}
	}
	WSACleanup();

	return 0;
}