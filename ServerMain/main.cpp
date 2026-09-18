#include <WinSock2.h>
#include <iostream>

#include "server.h"
#include "args.h"
#include "logger.h"

#pragma comment(lib, "Ws2_32.lib")

static constexpr const char* port = "port";
static constexpr const char* ip = "ip";
static constexpr const char* log_level = "log-level";

int main(int argc, char* argv[]) {
	Args::ArgsMap argsConfig{
		{ ip, Args::Arg{ Args::Type::string, std::string("127.0.0.1"), "IP of the server" } },
		{ port, Args::Arg{ Args::Type::integer, 8081, "Port of the server"} },
		{ log_level, Args::Arg{ Args::Type::string, std::string("info"), "Logging level"}},
	};
	Args::Commands commands{ 
		Args::Command{"run", "Runs the server"},
		Args::Command{"help", "Prints all arguments and commands"}
	};
	Args args{ std::move(argsConfig), std::move(commands) };
	auto errMsg = args.parse(argc, argv);
	if (!args.isValid()) {
		std::cout << errMsg << args.getDescription();
		return 0;
	}
	if (args.getCommand() == commandHelp || args.getCommand().empty()) {
		std::cout << args.getDescription();
		return 0;
	}
	auto loglvlstr = args.get<std::string>(log_level);
	auto lvl = logs::strToLvl(loglvlstr);
	server::logger.setLogLevel(lvl);
	WSADATA wsaData;
	WORD mVersionRequested = MAKEWORD(2, 2);
	int wsaError = WSAStartup(mVersionRequested, &wsaData);
	if (wsaError) {
		server::logger.logError(wsaError, " Error on WSA stratup\n");
		WSACleanup();
		return wsaError;
	}

	Server server{ args.get<std::string>(ip) , args.get<int>(port) };
	if (!server.open(4)) {
		server::logger.logError(" Error when opening server\n");
		return -1;
	}
	server.start();
	server.close();

	WSACleanup();
	return 0;
}