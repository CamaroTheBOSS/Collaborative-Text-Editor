#pragma once
#include <unordered_map>
#include <string>
#include <WinSock2.h>
#include <mutex>
#include <vector>
#include <set>
#include <concepts>

#include "engine.h"
#include "messages.h"
#include "server_document.h"
#include "response.h"
#include "authenticator.h"
#include "database.h"
#include "logging.h"

namespace server {
	template <typename T>
	concept ServerSiteDocumentType = std::is_same<ServerSiteDocument&, T>::value || std::is_same<ServerSiteDocument, T>::value;

	using SessionIt = std::unordered_map<std::string, ServerSiteDocument>::iterator;
	class Repository {
	public:
		Repository(server::Authenticator* auth);

		Repository(const Repository&) = delete;
		Repository& operator=(const Repository&) = delete;
		Repository(Repository&&);
		Repository& operator=(Repository&&);

		Response process(SOCKET client, msg::Buffer& buffer, bool authenticateUser = true);
		bool acCodeExists(const std::string& acCode);
		bool userFileExists(const std::string& username, const std::string& filename);
	private:
		struct ArgPack {
			SOCKET client;
			msg::Buffer& buffer;
			ServerSiteDocument* doc;
		};
		struct ClientUserData {
			std::string acCode;
			std::string username;
			std::string authToken;
		};
		ServerSiteDocument* findDoc(SOCKET client);
		Response processImpl(const msg::Type type, const ArgPack& argPack);
		Response createDoc(msg::Buffer& buffer);
		Response loadDoc(msg::Buffer& buffer);
		Response joinDoc(msg::Buffer& buffer);
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
		bool saveDocInDb(const ServerSiteDocument& doc);
		SessionIt getSessionWithDocId(const std::string& id);
		SessionIt getSessionWithAcCode(const std::string& acCode);

		template <ServerSiteDocumentType T>
		SessionIt createNewSession(const std::string& username, T&& doc) {
			auto acCode = random::Engine::get().getRandomString(6);
			auto session = acCodeToDocMap.emplace(acCode, std::move(doc));
			logger.logDebug("Created new session!");
			std::scoped_lock lock{acCodesLock, userFileCombinedLock};
			acCodeSet.insert(acCode);
			userFileCombinedSet.insert(username + "-" + doc.getFilename());
			return session.first;
		}
		void deleteSession(const std::string& username, const std::string& acCode, ServerSiteDocument& doc);
		void eraseClientFromSession(ServerSiteDocument& doc, const SOCKET client);
		bool addClientToSession(const SOCKET client, Authenticator::UserData& userAuthData, SessionIt session);

		std::unordered_map<SOCKET, ClientUserData> clientToUserData;
		std::unordered_map<std::string, ServerSiteDocument> acCodeToDocMap;

		// Authentication
		Authenticator* auth;
		std::chrono::seconds savingDocInterval{ 300 }; //5min 
		Database db{};


		std::mutex acCodesLock;
		std::set<std::string> acCodeSet;
		std::mutex userFileCombinedLock;
		std::set<std::string> userFileCombinedSet;
	};
}