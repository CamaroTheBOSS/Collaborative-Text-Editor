#pragma once
#include <WinSock2.h>
#include <thread>
#include <mutex>
#include <vector>
#include <memory>
#include <unordered_map>

#include "messages.h"
#include "repository.h"
#include "message_extractor.h"

class Worker {
public:
	Worker(const std::string& ip, const int port, server::Repository* repo);
	Worker(Worker&& worker) noexcept;
	Worker& operator=(Worker&& worker) noexcept;
	Worker(const Worker&) = delete;
	Worker& operator=(const Worker&) = delete;
private:
	void close();
	bool connectToMaster(const std::string& ip, const int port);
	void handleConnections();
	server::Response shutdownConnection(SOCKET client, msg::Buffer& buffer);
	server::Response processMsg(SOCKET client, msg::Buffer& buffer);
	void sendResponses(server::Response& response) const;
	void syncClientState(server::Response& response) const;
	
	friend class Server;
	bool opened = true;
	std::mutex connSetLock;
	FD_SET connections;

	sockaddr_in masterAddress = { 0 };
	SOCKET masterListener = INVALID_SOCKET;
	std::thread thread;

	server::Repository* repo;
	MessageExtractor extractor;
};
