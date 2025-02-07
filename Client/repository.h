#pragma once

#include "client_document.h"
#include "messages.h"
#include "screen_buffers.h"

namespace client {
	class Repository {
		friend class SyncTester;
	public:
		Repository(ClientSiteDocument& doc);
		bool processMsg(msg::Buffer& buffer);
	private:
		bool write(msg::Buffer& buffer);
		bool erase(msg::Buffer& buffer);
		bool move(msg::Buffer& buffer);
		bool sync(msg::Buffer& buffer);
		bool connectNewUser(msg::Buffer& buffer);
		bool disconnectUser(msg::Buffer& buffer);

		ClientSiteDocument& doc;
	};
}