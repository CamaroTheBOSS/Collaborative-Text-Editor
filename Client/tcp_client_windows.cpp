#include <WS2tcpip.h>
#include <chrono>

#include "tcp_client.h"

bool TCPClient::connectServer(const std::string& ip, const int port) {
    std::chrono::seconds waitTime{1};
    std::this_thread::sleep_for(waitTime);
    client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client == INVALID_SOCKET) {
        logger.logError(WSAGetLastError(), ": Error when creating socket for server connection!");
        closesocket(client);
        return false;
    }

    srvAddress.sin_family = AF_INET;
    srvAddress.sin_port = htons(port);
    std::wstring ipStr{ip.begin(), ip.end()};
    InetPton(AF_INET, ipStr.c_str(), &srvAddress.sin_addr.s_addr);
    if (connect(client, reinterpret_cast<SOCKADDR*>(&srvAddress), sizeof(srvAddress))) {
        logger.logError(WSAGetLastError(), ": Error when connecting to the server!");
        return false;
    }
    connected = true;
    recvThread = std::thread{ &TCPClient::recvMsg, this };
	return true;
}

bool TCPClient::disconnect() {
    connected = false;
    closesocket(client);
    if (recvThread.joinable()) {
        recvThread.join();
    }
    logger.logError("Disconnected from the server");
    return true;
}

bool TCPClient::isConnected() const {
    return connected;
}

msg::Buffer TCPClient::getNextMsg() {
    std::scoped_lock lock{recvQueueLock};
    if (recvQueue.empty()) {
        return msg::Buffer{0};
    }
    msg::Buffer msgBuffer = recvQueue.front();
    recvQueue.pop();
    return msgBuffer;
}

void TCPClient::recvMsg() {
    while (connected) {
        msg::Buffer buffer{128};
        buffer.size = recv(client, buffer.get(), buffer.capacity, 0);
        if (buffer.size < 0) {
            logger.logError(WSAGetLastError(), ": Recv error!");
            closesocket(client);
            return;
        }
        else if (buffer.size == 0) {
            logger.logError("Got disconnecting message from the server");
            closesocket(client);
            return;
        }
        auto messages = framer.extractMessages(buffer);
        std::scoped_lock lock{recvQueueLock};
        for (auto& msg : messages) {
            logger.logDebug("Put new message in queue with size", msg.size);
            recvQueue.push(std::move(msg));
        }
    }
}