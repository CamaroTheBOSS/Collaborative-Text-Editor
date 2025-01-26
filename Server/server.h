#pragma once
#include <WinSock2.h>
#include <string>
#include <thread>
#include <unordered_map>
#include <memory>

#include "worker.h"
#include "repository.h"

class Server {
public:
	Server(std::string ip, const int port);

	bool open(const int nWorkers);
	void start();
	bool close();
private:
	int selectWorker();
	void initWorkers(const int nWorkers);

	const std::string ip;
	const int port;
	SOCKET listenSocket = INVALID_SOCKET;
	sockaddr_in listenSocketAddress = { 0 };

	std::vector<Worker> workers;
	std::vector<SOCKET> notifiers;
	server::Repository repo;
};