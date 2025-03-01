#pragma once
#include <unordered_map>
#include <string>
#include <WinSock2.h>
#include <mutex>
#include <vector>

#include "messages.h"
#include "server_document.h"
#include "response.h"

namespace server {
	class Repository {
		friend class TestServer;
		friend class SyncTester;
	public:
		Response process(SOCKET client, msg::Buffer& buffer);
		std::string getLastAddedAcCode();
		std::string getLastDeletedAcCode();
	private:
		struct ArgPack {
			SOCKET client;
			msg::Buffer& buffer;
			ServerSiteDocument* doc;
		};
		ServerSiteDocument* findDoc(SOCKET client);
		Response createDoc(const SOCKET socket, msg::Buffer& buffer);
		Response loadDoc(const SOCKET socket, msg::Buffer& buffer);
		Response masterClose(msg::Buffer& buffer) const;
		//Response connectUserToDoc(const ArgPack& argPack);
		Response disconnectUserFromDoc(const ArgPack& argPack);
		//Response masterForwardConnect(const ArgPack& argPack);
		Response write(const ArgPack& argPack);
		Response erase(const ArgPack& argPack);
		Response moveHorizontal(const ArgPack& argPack);
		Response moveVertical(const ArgPack& argPack);
		Response moveTo(const ArgPack& argPack);
		Response moveSelectAll(const ArgPack& argPack);
		Response undoRedo(const ArgPack& argPack);
		Response replace(const ArgPack& argPack);
		void eraseFromMap(const ServerSiteDocument& doc, const SOCKET client);

		std::unordered_map<std::string, ServerSiteDocument> acCodeToDocMap;
		std::unordered_map<SOCKET, std::string> clientToAcCodeMap;
		std::string lastAddedAcCode;
		std::string lastDeletedAcCode;
	};
}