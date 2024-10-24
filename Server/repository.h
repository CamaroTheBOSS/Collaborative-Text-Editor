#pragma once
#include <unordered_map>
#include <string>
#include <WinSock2.h>
#include <mutex>
#include <vector>

#include "messages.h"
#include "document.h"

struct Response {
	msg::Buffer& buffer;
	std::vector<SOCKET> destinations;
};

struct Session {
	std::shared_ptr<Document> document;
	std::string accessCode;
	std::string filename;
};

class Repository {
public:
	Response process(SOCKET client, msg::Buffer& buffer);
	void connectUser(SOCKET client);
private:
	Response masterNotification(SOCKET client, msg::Buffer& buffer);
	Response write(SOCKET client, msg::Buffer& buffer);
	Response erase(SOCKET client, msg::Buffer& buffer);
	Response moveHorizontal(SOCKET client, msg::Buffer& buffer);
	Response moveVertical(SOCKET client, msg::Buffer& buffer);

	// TODO BETTER SYSTEM
	Document doc;
	std::unordered_map<SOCKET, int> clientToCursor;
	std::vector<SOCKET> connectedClients;
	std::mutex clientToCursorLock;
	std::mutex docLock;
};