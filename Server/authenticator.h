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
		struct UserData {
			std::string authToken;
			std::string username;
		};

		Database db{ dbRootDefault };

		Response loginUser(const ArgPack& args);
		Response logoutUser(const ArgPack& args);
		Response registerUser(const ArgPack& args);
		void addUser(const SOCKET client, const std::string& authToken, const std::string& username);
		bool checkIfUserIsActive(const std::string& username);

		std::mutex authMutex;
		std::unordered_map<SOCKET, UserData> clientToAuthToken;

		std::mutex activeUsersMutex;
		std::unordered_map<std::string, bool> activeUsers;
	};
}