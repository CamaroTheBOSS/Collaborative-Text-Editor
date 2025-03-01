#include <WinSock2.h>
#include <WS2tcpip.h>
#include "server.h"
#include "logging.h"

using namespace server;

constexpr int msgBufferSize = 6;
constexpr char closeType = static_cast<char>(msg::Type::masterClose);
constexpr char version = 1;
constexpr char closeMsgBuffer[msgBufferSize] = { 0, 0, 0, 2, closeType, version }; // (0002 = length)

Server::Server(std::string ip, const int port) :
	ip(ip),
	port(port) {
	listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSocket == INVALID_SOCKET) {
		logger.logError(WSAGetLastError(), ": Error when creating listening socket");
		return;
	}
	u_long mode = 1;
	int result = ioctlsocket(listenSocket, FIONBIO, &mode);
	if (result != NO_ERROR) {
		logger.logError(WSAGetLastError(), ": Error when setting nonblocking mode to listensocket");
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
	state = State::opened;
	return true;
}

void Server::start() {
	FD_ZERO(&unassignedConns);
	FD_SET(listenSocket, &unassignedConns);
	logger.logDebug("Listening for connections...");
	while (state == State::opened) {
		FD_SET conns = unassignedConns;
		int selectCount = select(0, &conns, nullptr, nullptr, nullptr);
		for (int i = 0; i < selectCount; i++) {
			SOCKET client = conns.fd_array[i];
			if (acceptConnection(client)) {
				continue;
			}
			auto msgs = extractor.extractMessages(client);
			for (auto& buffer : msgs) {
				msg::Type type;
				msg::OneByteInt version;
				msg::parse(buffer, 0, type, version);
				if (type == msg::Type::create) {
					buffer.replace(2, client);
					int worker = selectWorker();
					forwardConnection(client, buffer, worker);
				}
				else if (type == msg::Type::join) {
					std::string accessCode;
					buffer.replace(2, client);
					msg::parse(buffer, 6, accessCode);
					int worker = selectWorkerWithAcCode(accessCode);
					forwardConnection(client, buffer, worker);
				}
				else {
					auto response = processMsg(client, buffer);
					sendResponses(response);
				}
			}
		}
	}
	state = State::closed;
}

bool Server::acceptConnection(const SOCKET client) {
	if (client != listenSocket) {
		return false;
	}
	SOCKET newConnection = accept(listenSocket, nullptr, nullptr);
	if (newConnection == INVALID_SOCKET) {
		logger.logError(WSAGetLastError(), ": Error when accepting new connection");
		closesocket(newConnection);
		return false;
	}
	FD_SET(newConnection, &unassignedConns);
	return true;
}

bool Server::forwardConnection(const SOCKET client, const msg::Buffer& buffer, const int worker) {
	auto id = workers[worker].thread.get_id();
	{
		std::scoped_lock lock{workers[worker].connSetLock};
		FD_SET(client, &workers[worker].connections);
	}
	auto bufferWithSize = msg::enrich(buffer);
	int sendBytes = send(notifiers[worker], bufferWithSize.get(), bufferWithSize.size, 0);
	if (sendBytes < 0) {
		logger.logError(WSAGetLastError(), ": Error when notifying thread", id, "about new connection");
		return false;
	}
	logger.logDebug("Connection", client, "has been forwarded to thread", id);
	extractor.reset(client);
	FD_CLR(client, &unassignedConns);
	return true;
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

int Server::selectWorkerWithAcCode(const std::string& acCode) {
	for (int i = 0; i < workers.size(); i++) {
		std::scoped_lock lock{workers[i].acCodesLock};
		auto it = workers[i].acCodes.find(acCode);
		if (it != workers[i].acCodes.cend()) {
			return i;
		}
	}
	return 0;
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
	FD_SET set;
	FD_ZERO(&set);
	FD_SET(listenSocket, &set);
	for (int i = 0; i < nWorkers; i++) {
		Worker worker{ip, port};
		int socketCount = select(0, &set, nullptr, nullptr, nullptr);
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

void Server::sendResponses(server::Response& response) const {
	msg::Buffer msgWithSize = msg::enrich(response.buffer);
	for (const auto& dst : response.destinations) {
		int sendBytes = send(dst, msgWithSize.get(), msgWithSize.size, 0);
		if (sendBytes <= 0) {
			logger.logError("Error on sending data to", dst);
		}
	}
}

server::Response Server::processMsg(const SOCKET client, msg::Buffer& buffer) {
	if (buffer.size > 0) {
		return auth.process(client, buffer);
	}
	if (buffer.size < 0) {
		logger.logError(WSAGetLastError(), ": Error on receiving data from", client, "! Closing connection");
	}
	return shutdownConnection(client, buffer);
}

server::Response Server::shutdownConnection(const SOCKET client, msg::Buffer& buffer) {
	closesocket(client);
	shutdown(client, SD_SEND);
	logger.logDebug("Closing connection with", client);
	buffer.clear();
	msg::serializeTo(buffer, 0, msg::Type::logout, static_cast<msg::OneByteInt>(1));
	FD_CLR(client, &unassignedConns);
	return auth.process(client, buffer);
}