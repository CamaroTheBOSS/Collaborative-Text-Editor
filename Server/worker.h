#pragma once
#include <WinSock2.h>
#include <thread>
#include <mutex>
#include <set>

#include "messages.h"
#include "repository.h"
#include "message_extractor.h"
#include "authenticator.h"

class Worker {
public:
	friend class Server;
	Worker(const std::string& ip, const int port, server::Authenticator* auth);
	Worker(Worker&& worker) noexcept;
	Worker& operator=(Worker&& worker) noexcept;
	Worker(const Worker&) = delete;
	Worker& operator=(const Worker&) = delete;
	bool acCodeExistsInRepo(const std::string& acCode);
	bool userFileExistsInRepo(const std::string& username, const std::string& filename);
private:
	void close();
	bool connectToMaster(const std::string& ip, const int port);
	void handleConnections();
	server::Response shutdownConnection(SOCKET client, msg::Buffer& buffer);
	server::Response processMsg(SOCKET client, msg::Buffer& buffer);
	void sendResponses(server::Response& response) const;
	void syncClientState(server::Response& response) const;
	
	bool opened = true;
	std::mutex connSetLock;
	FD_SET connections;

	sockaddr_in masterAddress = { 0 };
	SOCKET masterListener = INVALID_SOCKET;
	std::thread thread;

	server::Repository repo;
	MessageExtractor extractor;
};
