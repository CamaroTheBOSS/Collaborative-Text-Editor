#include <WS2tcpip.h>
#include <iostream>

#include "worker.h"
#include "logging.h"

Worker::Worker(const std::string& ip, const int port, Repository* repo):
    repo(repo) {
    std::scoped_lock lock{connSetLock};
    FD_ZERO(&connections);
	thread = std::thread{ &Worker::connectToMaster, this, ip, port };
}

Worker::Worker(Worker&& worker) noexcept :
    connSetLock(),
    masterListener(std::move(worker.masterListener)),
    connections(std::move(worker.connections)),
    masterAddress(std::move(worker.masterAddress)),
    repo(worker.repo) {
    thread = std::thread{ &Worker::handleConnections, this };
}

bool Worker::connectToMaster(const std::string& ip, const int port) {
    // Create a communication pipe to master
    SOCKET masterListenerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (masterListenerSocket == INVALID_SOCKET) {
        logger.logError(WSAGetLastError(), ": Error on creating notify listener socket in thread ", std::this_thread::get_id());
        closesocket(masterListenerSocket);
        return false;
    }

    // Connect to master
    masterAddress.sin_family = AF_INET;
    masterAddress.sin_port = htons(port);
    std::wstring ipStr{ip.begin(), ip.end()};
    InetPton(AF_INET, ipStr.c_str(), &masterAddress.sin_addr.s_addr);
    if (connect(masterListenerSocket, reinterpret_cast<SOCKADDR*>(&masterAddress), sizeof(masterAddress))) {
        logger.logError(WSAGetLastError(), ": Error when connecting thread ", std::this_thread::get_id(), " to master");
        return false;
    }
    std::scoped_lock lock{connSetLock};
    FD_SET(masterListenerSocket, &connections);
    masterListener = masterListenerSocket;
    return true;
}

void Worker::handleConnections() {
    while (true) {
        FD_SET listenConnections;
        {
            std::scoped_lock lock{connSetLock};
            listenConnections = connections;
        }
        if (listenConnections.fd_count == 0) {
            continue;
        }
        int socketCount = select(0, &listenConnections, nullptr, nullptr, nullptr);
        for (int i = 0; i < socketCount; i++) {
            SOCKET client = listenConnections.fd_array[i];
            msg::Buffer recvBuffer = recvMsg(client);
            Response response = processMsg(client, recvBuffer);
            sendResponses(response);
        }
    }
}

msg::Buffer Worker::recvMsg(const SOCKET client) const {
    msg::Buffer recvBuff{128};
    recvBuff.size = recv(client, recvBuff.get(), recvBuff.capacity, 0);
    return recvBuff;
}

Response Worker::processMsg(const SOCKET client, msg::Buffer& buffer) {
    if (buffer.size > 0) {
        return repo->process(client, buffer);
    }
    if (buffer.size < 0) {
        logger.logError("Error on receiving data from ", client, "! Closing connection");
    }
    shutdownConnection(client);
    return Response{ buffer, {} };
}

void Worker::sendResponses(const Response& response) const {
    for (const auto& dst : response.destinations) {
        int sendBytes = send(dst, response.buffer.get(), response.buffer.size, 0);
        if (sendBytes <= 0) {
            logger.logError("Error on sending data to ", dst);
        }
    }
}

void Worker::shutdownConnection(const SOCKET client) {
    closesocket(client);
    shutdown(client, SD_SEND);
    std::scoped_lock lock{connSetLock};
    FD_CLR(client, &connections);
    logger.logDebug("Closing connection with ", client);
}
