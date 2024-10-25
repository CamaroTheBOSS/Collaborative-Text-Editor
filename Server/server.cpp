#include <WinSock2.h>
#include <WS2tcpip.h>
#include "server.h"
#include "logging.h"

Server::Server(std::string ip, const int port) :
	ip(ip),
	port(port) {
	listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSocket == INVALID_SOCKET) {
		logger.logError(WSAGetLastError(), ": Error when creating listening socket");
		return;
	}

	listenSocketAddress.sin_family = AF_INET;
	listenSocketAddress.sin_port = htons(port);
	std::wstring ipStr{ip.begin(), ip.end()};
	InetPton(AF_INET, ipStr.c_str(), &listenSocketAddress.sin_addr.s_addr);
	if (bind(listenSocket, reinterpret_cast<SOCKADDR*>(&listenSocketAddress), sizeof(listenSocketAddress)) == SOCKET_ERROR) {
		logger.logError(WSAGetLastError(), ": Error when binding listening socket");
	}
}

bool Server::open(const int nWorkers) {
	if (listen(listenSocket, SOMAXCONN)) {
		logger.logError(WSAGetLastError(), ": Error when starting listening");
		return false;
	}
	logger.logDebug("Server opened for listening.");
	initWorkers(nWorkers);
	return true;
}

void Server::start() {
	logger.logDebug("Listening for connections...");
	while (true) {
		SOCKET newConnection = accept(listenSocket, nullptr, nullptr);
		if (newConnection == INVALID_SOCKET) {
			logger.logError(WSAGetLastError(), ": Error when accepting new connection");
			closesocket(newConnection);
			continue;
		}
		int worker = selectWorker();
		auto id = workers[worker].thread.get_id();
		{
			std::scoped_lock lock{workers[worker].connSetLock};
			FD_SET(newConnection, &workers[worker].connections);
		}
		int sendBytes = send(notifiers[worker], "", 1, 0);
		if (sendBytes < 0) {
			logger.logError(WSAGetLastError(), ": Error when notifying thread", id, "about new connection");
			continue;
		}
		logger.logDebug("Connection", newConnection, "has been forwarded to thread", id);
	}
}

bool Server::close() {
	return true;
}

int Server::selectWorker() {
	unsigned int leastConns = INT_MAX;
	int leastConnsWorker = 0;
	for (int i = 0; i < workers.size(); i++) {
		std::scoped_lock lock{workers[i].connSetLock};
		if (workers[i].connections.fd_count < leastConns) {
			leastConns = workers[i].connections.fd_count;
			leastConnsWorker = i;
		}
	}
	return leastConnsWorker;
}

void Server::initWorkers(const int nWorkers) {
	workers.reserve(nWorkers);
	notifiers.reserve(nWorkers);
	for (int i = 0; i < nWorkers; i++) {
		Worker worker{ip, port, &repo};
		auto notifySocket = accept(listenSocket, nullptr, nullptr);
		if (notifySocket == INVALID_SOCKET) {
			closesocket(notifySocket);
			logger.logError(WSAGetLastError(), ": Error on opening notify socket to thread", worker.thread.get_id());
			continue;
		}
		if (worker.thread.joinable()) {
			worker.thread.join();
		}
		workers.push_back(std::move(worker));
		notifiers.push_back(notifySocket);
	}
	logger.logDebug("Created", workers.size(), "threads");
}