#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")

#include <iostream>
#include <chrono>
#include <random>
#include "action_scenarios.h"

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

int main(int argc, char* argv[]) {
	Args::ArgsMap argsConfig{
		{ ip, Args::Arg{ Args::Type::string, "IP of the server" } },
		{ port, Args::Arg{ Args::Type::integer, 8081, "Port of the server"} },
		{ login, Args::Arg{ Args::Type::string, "Login for the user"} },
		{ password, Args::Arg{ Args::Type::string, "Password for the user"} },
		{ acCode, Args::Arg{ Args::Type::string, "Access code to document"} },
		{ filename, Args::Arg{ Args::Type::string, std::string{"X"}, "Reserved"} },
	};
	Args::Commands commands{
		Args::Command{join, "Joins to the existing document session using provided access code", { ip, port, login, password, acCode }},
		Args::Command{registration, "Registers user in the server and terminating", { login, password }},
		Args::Command{help, "Prints all arguments and commands"},
	};
	Args args{ std::move(argsConfig), std::move(commands) };
	auto errMsg = args.parse(argc, argv);
	if (!args.isValid()) {
		std::cout << errMsg << args.getDescription();
		return 0;
	}
	auto command = args.getCommand();
	if (command == commandHelp || command == commandRun) {
		std::cout << args.getDescription();
		return 0;
	}

	WSADATA wsaData;
	WORD mVersionRequested = MAKEWORD(2, 2);
	int wsaError = WSAStartup(mVersionRequested, &wsaData);
	if (wsaError) {
		std::cout << wsaError << " Error on WSA stratup\n";
		WSACleanup();
		return 0;
	}

	Application app{ args.get<std::string>(ip) , args.get<int>(port) };
	if (command == registration) {
		// Focus on REGISTER menu option and click it
		goDownInMenuAndAccept(app, 1);
		passCredentials(app, args);
		return 0;
	}
	// Login
	doAndRender(app, ENTER);
	passCredentials(app, args);

	// Close information window, focus on Join option and click it
	doAndRender(app, ESC);
	goDownInMenuAndAccept(app, 3);

	// Pass accode
	auto& accessCode = args.get<std::string>(acCode);
	passStringToWindow(app, accessCode);
	doAndRender(app, ENTER);
	doAndRender(app, ESC);
	auto actionDone = false;
	bool render = false;
	while (true) {
		KeyPack key{0, false};
		key.keyCode = getRandomKey();
		actionDone = app.processChar(key);
		render = app.checkIncomingMessages();
		if (render || actionDone) {
			app.render();
		}
	}
	WSACleanup();

	return 0;
}