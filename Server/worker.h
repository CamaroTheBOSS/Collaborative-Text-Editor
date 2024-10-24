#pragma once
#include <WinSock2.h>
#include <thread>
#include <mutex>
#include <vector>
#include <memory>

#include "messages.h"
#include "repository.h"

class Worker {
public:
	Worker(const std::string& ip, const int port, Repository* repo);
	Worker(Worker&& worker) noexcept;
	Worker(const Worker&) = delete;
private:
	bool connectToMaster(const std::string& ip, const int port);
	void handleConnections();
	void shutdownConnection(SOCKET client);
	msg::Buffer recvMsg(SOCKET client) const;
	Response processMsg(SOCKET client, msg::Buffer& buffer);
	void sendResponses(const Response& response) const;
	
	friend class Server;
	std::mutex connSetLock;
	FD_SET connections;

	sockaddr_in masterAddress = { 0 };
	SOCKET masterListener = INVALID_SOCKET;
	std::thread thread;

	Repository* repo;
};
