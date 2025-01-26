#pragma once
#define _WINSOCKAPI_ 
#include <Windows.h>
#include <thread>
#include <queue>
#include <mutex>
#include <atomic>

#include "messages.h"
#include "logging.h"
#include "framer.h"

class TCPClient {
public:
	bool connectServer(const std::string& ip, const int port);
	bool disconnect();
	bool isConnected() const;
	msg::Buffer getNextMsg();
	template<typename... Args>
	bool sendMsg(Args&&... args) {
		msg::Buffer buffer{128};
		msg::serializeTo(buffer, 0, args...);
		msg::Buffer msgWithSize = msg::enrich(buffer);
		int sentBytes = send(client, msgWithSize.get(), msgWithSize.size, 0);
		if (sentBytes <= 0) {
			client::logger.logError(WSAGetLastError(), ": Send error!");
			return false;
		}
		client::logger.logDebug("Send message with args:", args...);
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
	Framer framer{4096};
};