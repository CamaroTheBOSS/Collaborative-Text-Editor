#include <WinSock2.h>
#include <WS2tcpip.h>
#include "server.h"
#include "logging.h"

using namespace server;

constexpr int msgBufferSize = 6;
constexpr char notifyType = static_cast<char>(msg::Type::masterNotification);
constexpr char closeType = static_cast<char>(msg::Type::masterClose);
constexpr char version = 1;
constexpr char notifyMsgBuffer[msgBufferSize] = { 0, 0, 0, 2, notifyType, version }; // (0002 = length)
constexpr char closeMsgBuffer[msgBufferSize] = { 0, 0, 0, 2, closeType, version }; // (0002 = length)

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
	state = State::opened;
	while (state == State::opened) {
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
		int sendBytes = send(notifiers[worker], notifyMsgBuffer, msgBufferSize, 0);
		if (sendBytes < 0) {
			logger.logError(WSAGetLastError(), ": Error when notifying thread", id, "about new connection");
			continue;
		}
		logger.logDebug("Connection", newConnection, "has been forwarded to thread", id);
	}
	state = State::closed;
}

bool Server::close() {
	logger.logDebug("Got signal for close. Closing server...");
	state = State::closing;
	bool success = true;
	SOCKET client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (client == INVALID_SOCKET) {
		logger.logError(WSAGetLastError(), ": Error when creating socket for server closing!");
		closesocket(client);
		success = false;
	}
	sockaddr_in srvAddress;
	srvAddress.sin_family = AF_INET;
	srvAddress.sin_port = htons(port);
	std::wstring ipStr{ip.begin(), ip.end()};
	InetPton(AF_INET, ipStr.c_str(), &srvAddress.sin_addr.s_addr);
	if (connect(client, reinterpret_cast<SOCKADDR*>(&srvAddress), sizeof(srvAddress))) {
		logger.logError(WSAGetLastError(), ": Error when notifying server for closing!");
		success = false;
	}
	if (int closed = closeWorkers(); closed < workers.size()) {
		logger.logError(WSAGetLastError(), ": Error when closing workers. Closed only", closed, "/", workers.size(), "|", workers.size() - closed, "dangling workers are present!");
		success = false;
	}
	closesocket(listenSocket);
	return success;
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

int Server::closeWorkers() {
	int closed = 0;
	for (int i = workers.size() - 1; i >= 0; i--) {
		auto id = workers[i].thread.get_id();
		int sendBytes = send(notifiers[i], closeMsgBuffer, msgBufferSize, 0);
		if (sendBytes < 0) {
			logger.logError(WSAGetLastError(), ": Error when notifying thread", id, "for close");
			continue;
		}
		if (workers[i].thread.joinable()) {
			workers[i].thread.join();
		}
		closed++;
	}
	return closed;
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