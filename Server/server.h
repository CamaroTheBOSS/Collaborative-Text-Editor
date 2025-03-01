#pragma once
#include <WinSock2.h>
#include <string>
#include <thread>
#include <unordered_map>
#include <memory>
#include <atomic>

#include "worker.h"
#include "repository.h"
#include "authenticator.h"

class Server {
public:
	Server(std::string ip, const int port);

	bool open(const int nWorkers);
	void start();
	bool close();
private:
	friend class SyncTester;
	enum class State {opened, closing, closed};
	bool forwardConnection(const SOCKET client, const msg::Buffer& buffer, const int worker);
	bool acceptConnection(const SOCKET client);
	int selectWorker();
	int selectWorkerWithAcCode(const std::string& acCode);
	void initWorkers(const int nWorkers);
	int closeWorkers();
	void sendResponses(server::Response& response) const;
	server::Response processMsg(const SOCKET client, msg::Buffer& buffer);
	server::Response shutdownConnection(const SOCKET client, msg::Buffer& buffer);

	std::atomic<State> state = State::closed;
	const std::string ip;
	const int port;
	SOCKET listenSocket = INVALID_SOCKET;
	sockaddr_in listenSocketAddress = { 0 };
	FD_SET unassignedConns = { 0 };

	std::vector<Worker> workers;
	std::vector<SOCKET> notifiers;
	MessageExtractor extractor;
	server::Authenticator auth;
};