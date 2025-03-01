#pragma once
#include <unordered_map>
#include <string>
#include <WinSock2.h>
#include <vector>
#include <mutex>

#include "messages.h"
#include "response.h"
#include "database.h"

namespace server {
	class Authenticator {
	public:
		Response process(SOCKET client, msg::Buffer& buffer);
		void clearUser(SOCKET client);
		std::string getAuthToken(SOCKET client);
	private:
		struct ArgPack {
			SOCKET client;
			msg::Buffer& buffer;
		};

		Database db{ dbRootDefault };

		Response loginUser(const ArgPack& args);
		Response logoutUser(const ArgPack& args);
		Response registerUser(const ArgPack& args);

		std::mutex authMutex;
		std::unordered_map<SOCKET, std::string> clientToAuthToken;
	};
}