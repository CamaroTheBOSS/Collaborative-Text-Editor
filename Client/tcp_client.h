#pragma once
#define _WINSOCKAPI_ 
#include <Windows.h>
#include <thread>
#include <queue>
#include <mutex>
#include <atomic>

#include "messages.h"
#include "logging.h"

class TCPClient {
public:
	bool connectServer(const std::string& ip, const int port);
	bool disconnect();
	msg::Buffer getNextMsg();
	template<typename... Args>
	bool sendMsg(Args&&... args) {
		msg::Buffer buffer{128};
		msg::serializeTo(buffer, 0, args...);
		int sentBytes = send(client, buffer.get(), buffer.size, 0);
		if (sentBytes <= 0) {
			logger.logError(WSAGetLastError(), ": Send error!");
			return false;
		}
		logger.logDebug("Send message with args:", args...);
		return true;
	}	

private:
	void recvMsg();

	SOCKET client = INVALID_SOCKET;
	sockaddr_in srvAddress = { 0 };
	
	std::thread recvThread;
	std::queue<msg::Buffer> recvQueue;
	std::mutex recvQueueLock;
	std::atomic_bool connected;
};