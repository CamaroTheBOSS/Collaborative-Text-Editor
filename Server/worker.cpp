#include <WS2tcpip.h>
#include <iostream>

#include "worker.h"
#include "logging.h"

using namespace server;

constexpr int defaultBuffSize = 128;

Worker::Worker(const std::string& ip, const int port, server::Repository* repo):
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

Worker& Worker::operator=(Worker&& worker) noexcept {
    masterListener = std::move(worker.masterListener);
    connections = std::move(worker.connections);
    masterAddress = std::move(worker.masterAddress);
    repo = std::move(worker.repo);
    thread = std::move(worker.thread);
    return *this;
}

bool Worker::connectToMaster(const std::string& ip, const int port) {
    // Create a communication pipe to master
    SOCKET masterListenerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (masterListenerSocket == INVALID_SOCKET) {
        logger.logError(WSAGetLastError(), ": Error on creating notify listener socket in thread", std::this_thread::get_id());
        closesocket(masterListenerSocket);
        return false;
    }

    // Connect to master
    masterAddress.sin_family = AF_INET;
    masterAddress.sin_port = htons(port);
    std::wstring ipStr{ip.begin(), ip.end()};
    InetPton(AF_INET, ipStr.c_str(), &masterAddress.sin_addr.s_addr);
    if (connect(masterListenerSocket, reinterpret_cast<SOCKADDR*>(&masterAddress), sizeof(masterAddress))) {
        logger.logError(WSAGetLastError(), ": Error when connecting thread", std::this_thread::get_id(), "to master");
        return false;
    }
    std::scoped_lock lock{connSetLock};
    FD_SET(masterListenerSocket, &connections);
    masterListener = masterListenerSocket;
    return true;
}

void Worker::handleConnections() {
    while (opened) {
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
            auto msgBuffers = extractor.extractMessages(client);
            for (auto& msgBuffer : msgBuffers) {
                server::Response response = processMsg(client, msgBuffer);
                if (response.msgType == msg::Type::sync) {
                    syncClientState(response);
                }
                if (response.msgType == msg::Type::masterClose) {
                    opened = false;
                }
                sendResponses(response);
            }
        }
    }
    close();
}

void Worker::close() {
    std::scoped_lock lock{connSetLock};
    for (int i = 0; i < connections.fd_count; i++) {
        closesocket(connections.fd_array[i]);
    }
    closesocket(masterListener);
}

server::Response Worker::processMsg(const SOCKET client, msg::Buffer& buffer) {
    if (buffer.size > 0) {
        return repo->process(client, buffer);
    }
    if (buffer.size < 0) {
        logger.logError(WSAGetLastError(), ": Error on receiving data from", client, "! Closing connection");
    }
    return shutdownConnection(client, buffer);
}

void Worker::syncClientState(server::Response& response) const {
    SOCKET lastConnectedClient = response.destinations[response.destinations.size() - 1];
    msg::Buffer msgWithSize = msg::enrich(response.buffer);
    int sendBytes = send(lastConnectedClient, msgWithSize.get(), msgWithSize.size, 0);
    if (sendBytes <= 0) {
        logger.logError("Error on sending data to", lastConnectedClient);
    }
    response.destinations.pop_back();
    response.buffer.clear();
    msg::serializeTo(response.buffer, 0, msg::Type::connect, static_cast<msg::OneByteInt>(1));
}

void Worker::sendResponses(server::Response& response) const {
    msg::Buffer msgWithSize = msg::enrich(response.buffer);
    for (const auto& dst : response.destinations) {
        int sendBytes = send(dst, msgWithSize.get(), msgWithSize.size, 0);
        if (sendBytes <= 0) {
            logger.logError("Error on sending data to", dst);
        }
    }
}

server::Response Worker::shutdownConnection(const SOCKET client, msg::Buffer& buffer) {
    closesocket(client);
    shutdown(client, SD_SEND);
    logger.logDebug("Closing connection with", client);
    buffer.clear();
    msg::serializeTo(buffer, 0, msg::Type::disconnect, static_cast<msg::OneByteInt>(1));
    std::scoped_lock lock{connSetLock};
    FD_CLR(client, &connections);
    return repo->process(client, buffer);
}
