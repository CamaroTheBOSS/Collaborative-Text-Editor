#pragma once

#include "client_document.h"
#include "messages.h"
#include "screen_buffers.h"

namespace client {
	class Repository {
		friend class SyncTester;
	public:
		bool processMsg(ClientSiteDocument& doc, msg::Buffer& buffer);
		std::string getAcCode() const;
		std::string getAuthToken() const;
		std::string getLastError();
	private:
		bool write(ClientSiteDocument& doc, msg::Buffer& buffer);
		bool erase(ClientSiteDocument& doc, msg::Buffer& buffer);
		bool move(ClientSiteDocument& doc, msg::Buffer& buffer);
		bool sync(ClientSiteDocument& doc, msg::Buffer& buffer);
		bool connectNewUser(ClientSiteDocument& doc, msg::Buffer& buffer);
		bool disconnectUser(ClientSiteDocument& doc, msg::Buffer& buffer);
		bool replace(ClientSiteDocument& doc, msg::Buffer& buffer);

		std::string acCode;
		std::string authToken;
		std::string lastError;
	};
}