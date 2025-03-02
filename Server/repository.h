#pragma once
#include <unordered_map>
#include <string>
#include <WinSock2.h>
#include <mutex>
#include <vector>

#include "messages.h"
#include "server_document.h"
#include "response.h"
#include "authenticator.h"
#include "database.h"

namespace server {
	class Repository {
		friend class TestServer;
		friend class SyncTester;
	public:
		Repository(server::Authenticator* auth);
		Response process(SOCKET client, msg::Buffer& buffer, bool authenticateUser = true);
		std::string getLastAddedAcCode();
		std::string getLastDeletedAcCode();
	private:
		struct ArgPack {
			SOCKET client;
			msg::Buffer& buffer;
			ServerSiteDocument* doc;
		};
		ServerSiteDocument* findDoc(SOCKET client);
		Response processImpl(const msg::Type type, const ArgPack& argPack);
		Response createDoc(msg::Buffer& buffer);
		Response loadDoc( msg::Buffer& buffer);
		Response masterClose(msg::Buffer& buffer) const;
		Response disconnectUserFromDoc(const ArgPack& argPack);
		Response write(const ArgPack& argPack);
		Response erase(const ArgPack& argPack);
		Response moveHorizontal(const ArgPack& argPack);
		Response moveVertical(const ArgPack& argPack);
		Response moveTo(const ArgPack& argPack);
		Response moveSelectAll(const ArgPack& argPack);
		Response undoRedo(const ArgPack& argPack);
		Response replace(const ArgPack& argPack);
		void eraseFromMap(const ServerSiteDocument& doc, const SOCKET client);
		std::string addToAuthMap(const SOCKET client);
		std::string saveDocInDb(const ServerSiteDocument& doc);

		std::unordered_map<std::string, ServerSiteDocument> acCodeToDocMap;
		std::unordered_map<SOCKET, std::string> clientToAcCodeMap;
		std::string lastAddedAcCode;
		std::string lastDeletedAcCode;

		// Authentication
		Authenticator* auth;
		std::unordered_map<SOCKET, Authenticator::UserData> clientToUser;
		std::chrono::seconds savingDocInterval{ 1 }; //5min 
		Database db{};
	};
}