#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")

#include <iostream>
#include <chrono>

#include "action_scenarios.h"

int main(int argc, char* argv[]) {
	Args::ArgsMap argsConfig{
		{ ip, Args::Arg{ Args::Type::string, "IP of the server" } },
		{ port, Args::Arg{ Args::Type::integer, 8081, "Port of the server"} },
		{ login, Args::Arg{ Args::Type::string, "Login for the user"} },
		{ password, Args::Arg{ Args::Type::string, "Password for the user"} },
		{ acCode, Args::Arg{ Args::Type::string, "Access code to document"} },
		{ filename, Args::Arg{ Args::Type::string, "Filename of created document"} },
	};
	Args::Commands commands{
		Args::Command{run, "Run the software", { ip, port }},
		Args::Command{join, "Joins to the existing document session using provided access code", { ip, port, login, password, acCode }},
		Args::Command{create, "Create new session with new document", { ip, port, login, password, filename }},
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
		scenarioRegisterUser(app, args);
		WSACleanup();
		return 0;
	}
	else if (command == join) {
		scenarioJoinDoc(app, args);
	}
	else if (command == create) {
		scenarioCreateDoc(app, args);
	}
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