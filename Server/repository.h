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
	msg::Type msgType;
};

struct Session {
	std::shared_ptr<Document> document;
	std::string accessCode;
	std::string filename;
};

class Repository {
public:
	Response process(SOCKET client, msg::Buffer& buffer);
private:
	Response connectUserToDoc(SOCKET client, msg::Buffer& buffer);
	Response disconnectUserFromDoc(SOCKET client, msg::Buffer& buffer);
	Response masterNotification(SOCKET client, msg::Buffer& buffer) const;
	Response write(SOCKET client, msg::Buffer& buffer);
	Response erase(SOCKET client, msg::Buffer& buffer);
	Response moveHorizontal(SOCKET client, msg::Buffer& buffer);
	Response moveVertical(SOCKET client, msg::Buffer& buffer);
	Response moveSelectAll(SOCKET client, msg::Buffer& buffer);
	int findClient(SOCKET client);

	// TODO BETTER SYSTEM
	Document doc{"", 0, 0};
	std::vector<SOCKET> connectedClients;
	std::mutex connectedClientsLock;
	std::mutex docLock;
};